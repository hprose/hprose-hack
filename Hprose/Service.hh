<?hh // strict
/**********************************************************\
|                                                          |
|                          hprose                          |
|                                                          |
| Official WebSite: http://www.hprose.com/                 |
|                   http://www.hprose.org/                 |
|                                                          |
\**********************************************************/

/**********************************************************\
 *                                                        *
 * Hprose/Service.hh                                      *
 *                                                        *
 * hprose service library for hack.                       *
 *                                                        *
 * LastModified: Feb 27, 2015                             *
 * Author: Ma Bingyao <andot@hprose.com>                  *
 *                                                        *
\**********************************************************/

namespace Hprose {
    class RemoteCall {
        public function __construct(public mixed $func,
                                    public ResultMode $mode,
                                    public ?bool $simple) {}
    }
    abstract class Service {
        private static array<string> $magic_methods = array(
            "__construct",
            "__destruct",
            "__call",
            "__callStatic",
            "__get",
            "__set",
            "__isset",
            "__unset",
            "__sleep",
            "__wakeup",
            "__toString",
            "__invoke",
            "__set_state",
            "__clone"
        );
        protected static Map<int, string> $errorTable = Map {
            E_ERROR => 'Error',
            E_WARNING => 'Warning',
            E_PARSE => 'Parse Error',
            E_NOTICE => 'Notice',
            E_CORE_ERROR => 'Core Error',
            E_CORE_WARNING => 'Core Warning',
            E_COMPILE_ERROR => 'Compile Error',
            E_COMPILE_WARNING => 'Compile Warning',
            E_DEPRECATED => 'Deprecated',
            E_USER_ERROR => 'User Error',
            E_USER_WARNING => 'User Warning',
            E_USER_NOTICE => 'User Notice',
            E_USER_DEPRECATED => 'User Deprecated',
            E_STRICT => 'Runtime Notice',
            E_RECOVERABLE_ERROR  => 'Catchable Fatal Error'
        };
        private Map<string, RemoteCall> $calls = Map {};
        private Vector<string> $names = Vector {};
        private Vector<Filter> $filters = Vector {};
        private bool $simple = false;
        protected bool $debug = false;
        protected int $error_types = E_ALL & ~E_NOTICE;
        public ?(function(string, array<mixed>, bool, \stdClass): void) $onBeforeInvoke = null;
        public ?(function(string, array<mixed>, bool, mixed, \stdClass): void) $onAfterInvoke = null;
        public ?(function(string, \stdClass): void) $onSendError = null;

        protected function inputFilter(string $data,
                                       \stdClass $context): string {
            $count = count($this->filters);
            for ($i = $count - 1; $i >= 0; $i--) {
                $data = $this->filters[$i]->inputFilter($data, $context);
            }
            return $data;
        }
        protected function outputFilter(string $data,
                                        \stdClass $context): string {
            $count = count($this->filters);
            for ($i = 0; $i < $count; $i++) {
                $data = $this->filters[$i]->outputFilter($data, $context);
            }
            return $data;
        }
        protected function sendError(string $error,
                                     \stdClass $context): string {
            if ($this->onSendError !== null) {
                $sendError = $this->onSendError;
                $sendError($error, $context);
            }
            @ob_clean();
            $data = Tags::TagError .
                    \hprose_serialize_string($error) .
                    Tags::TagEnd;
            return $this->outputFilter($data, $context);
        }
        protected function doInvoke(StringStream $input,
                                    \stdClass $context): string {
            $output = new StringStream();
            do {
                $name = hprose_unserialize_string_with_stream($input, true);
                $alias = strtolower($name);
                if ($this->calls->contains($alias)) {
                    $call = $this->calls[$alias];
                }
                elseif ($this->calls->contains('*')) {
                    $call = $this->calls['*'];
                }
                else {
                    throw new \Exception("Can't find this function " . $name . "().");
                }
                $mode = $call->mode;
                $simple = $call->simple;
                if ($simple === null) {
                    $simple = $this->simple;
                }
                $args = array();
                $byref = false;
                $tag = $input->getc();
                if ($tag == Tags::TagList) {
                    $args = \hprose_unserialize_list_with_stream($input)->toArray();
                    $tag = $input->getc();
                    if ($tag == Tags::TagTrue) {
                        $byref = true;
                        $tag = $input->getc();
                    }
                }
                if (($tag != Tags::TagEnd) && ($tag != Tags::TagCall)) {
                    throw new \Exception('Unknown tag: ' . $tag . "\r\n" .
                                         'with following data: ' . $input->toString());
                }
                if ($this->onBeforeInvoke !== null) {
                    $beforeInvoke = $this->onBeforeInvoke;
                    $beforeInvoke($name, $args, $byref, $context);
                }
                if ($this->calls->contains('*') &&
                    $call === $this->calls['*']) {
                    $args = array($name, $args);
                }
                $result = call_user_func_array($call->func, $args);
                if ($this->onAfterInvoke !== null) {
                    $afterInvoke = $this->onAfterInvoke;
                    $afterInvoke($name, $args, $byref, $result, $context);
                }
                // some service functions/methods may echo content, we need clean it
                @ob_clean();
                if ($mode == ResultMode::RawWithEndTag) {
                    return $this->outputFilter($result, $context);
                }
                elseif ($mode == ResultMode::Raw) {
                    $output->write($result);
                }
                else {
                    $output->write(Tags::TagResult);
                    if ($mode == ResultMode::Serialized) {
                        $output->write($result);
                    }
                    else {
                        $output->write(\hprose_serialize($result, $simple));
                    }
                    if ($byref) {
                        $output->write(Tags::TagArgument .
                                       \hprose_serialize_list($args, $simple));
                    }
                }
            } while ($tag == Tags::TagCall);
            $output->write(Tags::TagEnd);
            return $this->outputFilter($output->toString(), $context);
        }
        protected function doFunctionList(\stdClass $context): string {
            $data = Tags::TagFunctions .
                    \hprose_serialize_list($this->names, true) .
                    Tags::TagEnd;
            return $this->outputFilter($data, $context);
        }
        private static function getDeclaredOnlyMethods(string $class): array<string> {
            $result = get_class_methods($class);
            if ($parent_class = get_parent_class($class)) {
                $inherit = get_class_methods($parent_class);
                $result = array_diff($result, $inherit);
            }
            $result = array_diff($result, self::$magic_methods);
            return $result;
        }
        public function addMissingFunction(mixed $func,
                                       ResultMode $mode = ResultMode::Normal,
                                       ?bool $simple = null): void {
            $this->addFunction($func, '*', $mode, $simple);
        }
        public function addFunction(mixed $func,
                                    string $alias = '',
                                    ResultMode $mode = ResultMode::Normal,
                                    ?bool $simple = null): void {
            if (!is_callable($func)) {
                throw new \Exception('Argument func is not callable.');
            }
            if ($alias == '') {
                if (is_string($func)) {
                    $alias = $func;
                }
                elseif (is_array($func)) {
                    $alias = $func[1];
                }
                else {
                    throw new \Exception('alias must be a string.');
                }
            }
            $name = strtolower($alias);
            if (!$this->calls->contains($name)) {
                $this->names->add($alias);
            }
            $this->calls[$name] = new RemoteCall($func, $mode, $simple);
        }
        public function addFunctions(array<mixed> $funcs,
                                     array<string> $aliases = array(),
                                     ResultMode $mode = ResultMode::Normal,
                                     ?bool $simple = null): void {
            $count = count($aliases);
            if ($count == 0) {
                foreach ($funcs as $func) {
                    $this->addFunction($func, '', $mode, $simple);
                }
            }
            elseif ($count == count($funcs)) {
                foreach ($funcs as $i => $func) {
                    $this->addFunction($func, $aliases[$i], $mode, $simple);
                }
            }
            else {
                throw new \Exception('The count of functions is not matched with aliases');
            }
        }
        public function addMethod(string $methodname,
                                  mixed $belongto,
                                  string $alias = '',
                                  ResultMode $mode = ResultMode::Normal,
                                  ?bool $simple = null): void {
            $func = array($belongto, $methodname);
            $this->addFunction($func, $alias, $mode, $simple);
        }
        public function addMethods(array<string> $methods,
                                   mixed $belongto,
                                   mixed $aliases = '',
                                   ResultMode $mode = ResultMode::Normal,
                                   ?bool $simple = null): void {
            if ($aliases === null || count($aliases) == 0) {
                $aliases = '';
            }
            $_aliases = array();
            if (is_string($aliases)) {
                $aliasPrefix = $aliases;
                if ($aliasPrefix !== '') {
                    $aliasPrefix .= '_';
                }
                foreach ($methods as $k => $method) {
                    $_aliases[$k] = $aliasPrefix . $method;
                }
            }
            elseif (is_array($aliases)) {
                $_aliases = $aliases;
            }
            elseif ($aliases instanceof \Indexish) {
                $_aliases = $aliases;
            }
            if (count($methods) != count($_aliases)) {
                throw new \Exception('The count of methods is not matched with aliases');
            }
            foreach($methods as $k => $method) {
                $func = array($belongto, $method);
                $this->addFunction($func, $_aliases[$k], $mode, $simple);
            }
        }
        public function addInstanceMethods(mixed $object,
                                        string $class = '',
                                        string $aliasPrefix = '',
                                        ResultMode $mode = ResultMode::Normal,
                                        ?bool $simple = null): void {
            if ($class == '') {
                $class = get_class($object);
            }
            $this->addMethods(self::getDeclaredOnlyMethods($class),
                              $object, $aliasPrefix, $mode, $simple);
        }
        public function addClassMethods(string $class,
                                        string $execclass = '',
                                        string $aliasPrefix = '',
                                        ResultMode $mode = ResultMode::Normal,
                                        ?bool $simple = null): void {
            if ($execclass == '') {
                $execclass = $class;
            }
            $this->addMethods(self::getDeclaredOnlyMethods($class),
                              $execclass, $aliasPrefix, $mode, $simple);
        }
        public function add(...): void {
            $args_num = func_num_args();
            $args = func_get_args();
            switch ($args_num) {
                case 1: {
                    if (is_callable($args[0])) {
                        $this->addFunction($args[0]);
                        return;
                    }
                    elseif (is_array($args[0])) {
                        $this->addFunctions($args[0]);
                        return;
                    }
                    elseif (is_object($args[0])) {
                        $this->addInstanceMethods($args[0]);
                        return;
                    }
                    elseif (is_string($args[0])) {
                        $this->addClassMethods($args[0]);
                        return;
                    }
                    break;
                }
                case 2: {
                    if (is_callable($args[0]) && is_string($args[1])) {
                        $this->addFunction($args[0], $args[1]);
                        return;
                    }
                    elseif (is_string($args[0])) {
                        if (is_string($args[1]) && !is_callable(array($args[1], $args[0]))) {
                            if (class_exists($args[1])) {
                                $this->addClassMethods($args[0], $args[1]);
                            }
                            else {
                                $this->addClassMethods($args[0], '', $args[1]);
                            }
                        }
                        else {
                            $this->addMethod($args[0], $args[1]);
                        }
                        return;
                    }
                    elseif (is_array($args[0])) {
                        if (is_array($args[1])) {
                            $this->addFunctions($args[0], $args[1]);
                        }
                        else {
                            $this->addMethods($args[0], $args[1]);
                        }
                        return;
                    }
                    elseif (is_object($args[0])) {
                        $this->addInstanceMethods($args[0], $args[1]);
                        return;
                    }
                    break;
                }
                case 3: {
                    if (is_callable($args[0]) && $args[1] == '' && is_string($args[2])) {
                        $this->addFunction($args[0], $args[2]);
                        return;
                    }
                    elseif (is_string($args[0]) && is_string($args[2])) {
                        if (is_string($args[1]) && !is_callable(array($args[0], $args[1]))) {
                            $this->addClassMethods($args[0], $args[1], $args[2]);
                        }
                        else {
                            $this->addMethod($args[0], $args[1], $args[2]);
                        }
                        return;
                    }
                    elseif (is_array($args[0])) {
                        if ($args[1] == '' && is_array($args[2])) {
                            $this->addFunctions($args[0], $args[2]);
                        }
                        else {
                            $this->addMethods($args[0], $args[1], $args[2]);
                        }
                        return;
                    }
                    elseif (is_object($args[0])) {
                        $this->addInstanceMethods($args[0], $args[1], $args[2]);
                        return;
                    }
                    break;
                }
                throw new \Exception('Wrong arguments');
            }
        }
        public function isDebugEnabled(): bool {
            return $this->debug;
        }
        public function setDebugEnabled(bool $enable = true): void {
            $this->debug = $enable;
        }
        public function getFilter(): ?Filter {
            if (count($this->filters) === 0) {
                return null;
            }
            return $this->filters[0];
        }
        public function setFilter(Filter $filter): void {
            $this->filters->clear();
            if ($filter !== null) {
                $this->filters->add($filter);
            }
        }
        public function addFilter(Filter $filter): void {
            $this->filters->add($filter);
        }
        public function removeFilter(Filter $filter): bool {
            $i = $this->filters->linearSearch($filter);
            if ($i === -1) {
                return false;
            }
            $this->filters->removeKey($i);
            return true;
        }
        public function getSimpleMode(): bool {
            return $this->simple;
        }
        public function setSimpleMode(bool $simple = true): void {
            $this->simple = $simple;
        }
        public function getErrorTypes(): int {
            return $this->error_types;
        }
        public function setErrorTypes(int $error_types): void {
            $this->error_types = $error_types;
        }
        public function defaultHandle(string $request,
                                      \stdClass $context): string {
            $input = new StringStream($this->inputFilter($request, $context));
            try {
                switch ($input->getc()) {
                    case Tags::TagCall:
                        return $this->doInvoke($input, $context);
                    case Tags::TagEnd:
                        return $this->doFunctionList($context);
                    default:
                        throw new \Exception("Wrong Request: \r\n" . $request);
                }
            }
            catch (\Exception $e) {
                $error = $e->getMessage();
                if ($this->debug) {
                    $error .= "\nfile: " . $e->getFile() .
                              "\nline: " . $e->getLine() .
                              "\ntrace: " . $e->getTraceAsString();
                }
                return $this->sendError($error, $context);
            }
        }
        public abstract function handle(): void;
        public function start(): void {
            $this->handle();
        }
    }
}

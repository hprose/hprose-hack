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
 * Hprose/Client.hh                                       *
 *                                                        *
 * hprose client library for hack.                        *
 *                                                        *
 * LastModified: Mar 8, 2015                              *
 * Author: Ma Bingyao <andot@hprose.com>                  *
 *                                                        *
\**********************************************************/

namespace Hprose {
    class Proxy {
        private Client $client;
        private string $namespace;
        public function __construct(Client $client, string $namespace = '') {
            $this->client = $client;
            $this->namespace = $namespace;
        }
        public function __destruct(): void {
        }
        public function __call(string $name, array<mixed> $arguments): mixed {
            $name = $this->namespace . $name;
            $n = count($arguments);
            if ($n > 0) {
                if (is_callable($arguments[$n - 1])) {
                    $callback = array_pop($arguments);
                    $this->client->invoke($name, new Vector($arguments), false, ResultMode::Normal, false, $callback);
                    return;
                }
            }
            return $this->client->invoke($name, new Vector($arguments));
        }
        public function __get(string $name): mixed {
            return new Proxy($this->client, $this->namespace . $name . '_');
        }
    }

    abstract class Client extends Proxy {
        protected string $url;
        private Vector<Filter> $filters;
        private bool $simple;
        protected function sendAndReceive(string $request): string {
           throw new \Exception("This client can't support synchronous invoke.");
        }
        protected function asyncSendAndReceive(string $request, (function(string, ?\Exception): void) $callback): void {
            throw new \Exception("This client can't support asynchronous invoke.");
        }
        public function __construct(string $url = '') {
            $this->url = $url;
            $this->filters = Vector {};
            $this->simple = false;
            // UNSAFE
            parent::__construct($this, '');
        }
        public function useService(string $url = '', string $namespace = ''): mixed {
            if ($url) {
                $this->url = $url;
            }
            if ($namespace) {
                $namespace .= "_";
            }
            return new Proxy($this, $namespace);
        }
        private function doOutput(string $name, Vector<mixed> $args, bool $byref, ?bool $simple, \stdClass $context): string {
            if ($simple === null) {
                $simple = $this->simple;
            }
            $stream = new BytesIO(Tags::TagCall);
            $writer = new Writer($stream, $simple);
            $writer->writeString($name);
            if (count($args) > 0 || $byref) {
                $writer->reset();
                $writer->writeList($args);
                if ($byref) {
                    $writer->writeBoolean(true);
                }
            }
            $stream->write(Tags::TagEnd);
            $request = $stream->toString();
            $count = count($this->filters);
            for ($i = 0; $i < $count; $i++) {
                $request = $this->filters[$i]->outputFilter($request, $context);
            }
            $stream->close();
            return $request;
        }
        private function doInput(string $response, Vector<mixed> $args, ResultMode $mode, \stdClass $context): mixed {
            $count = count($this->filters);
            for ($i = $count - 1; $i >= 0; $i--) {
                $response = $this->filters[$i]->inputFilter($response, $context);
            }
            if ($mode == ResultMode::RawWithEndTag) {
                return $response;
            }
            if ($mode == ResultMode::Raw) {
                return substr($response, 0, -1);
            }
            $stream = new BytesIO($response);
            $reader = new Reader($stream);
            $result = null;
            while (($tag = $stream->getc()) !== Tags::TagEnd) {
                switch ($tag) {
                    case Tags::TagResult:
                        if ($mode == ResultMode::Serialized) {
                            $result = $reader->readRaw()->toString();
                        }
                        else {
                            $reader->reset();
                            $result = $reader->unserialize();
                        }
                        break;
                    case Tags::TagArgument:
                        $reader->reset();
                        $_args = $reader->readList();
                        if ($_args instanceof Vector) {
                            $n = min(count($_args), count($args));
                            for ($i = 0; $i < $n; $i++) {
                                $args[$i] = $_args[$i];
                            }
                        }
                        break;
                    case Tags::TagError:
                        $reader->reset();
                        throw new \Exception((string)$reader->readString());
                        break;
                    default:
                        throw new \Exception("Wrong Response: \r\n" . $response);
                        break;
                }
            }
            return $result;
        }
        public function invoke(string $name, Vector<mixed> $args = Vector {}, bool $byref = false, ResultMode $mode = ResultMode::Normal, ?bool $simple = null, mixed $callback = null): mixed {
            $context = new \stdClass();
            $context->client = $this;
            $context->userdata = new \stdClass();
            $request = $this->doOutput($name, $args, $byref, $simple, $context);
            if (is_callable($callback)) {
                $this->asyncSendAndReceive($request, ($response, $error) ==> {
                    $result = null;
                    $callback = new \ReflectionFunction($callback);
                    $n = $callback->getNumberOfParameters();
                    if ($n === 3) {
                        if ($error === null) {
                            try {
                                $result = $this->doInput($response, $args, $mode, $context);
                            }
                            catch (\Exception $e) {
                                $error = $e;
                            }
                        }
                        $callback->invoke($result, $args, $error);
                    }
                    else {
                        if ($error !== null) throw $error;
                        $result = $this->doInput($response, $args, $mode, $context);
                        switch($n) {
                            case 0:
                                $callback->invoke(); break;
                            case 1:
                                $callback->invoke($result); break;
                            case 2:
                                $callback->invoke($result, $args); break;
                        }
                    }
                });
            }
            else {
                $response = $this->sendAndReceive($request);
                return $this->doInput($response, $args, $mode, $context);
            }
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
    }

}

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
 * LastModified: Feb 25, 2015                             *
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
        public function __call(string $name, array<mixed> $arguments): mixed {
            $name = $this->namespace . $name;
            return $this->client->invoke($name, new Vector($arguments));
        }
        public function __get(string $name): mixed {
            return new Proxy($this->client, $this->namespace . $name . '_');
        }
    }

    abstract class Client {
        protected string $url;
        private Vector<Filter> $filters;
        private bool $simple;
        protected abstract function sendAndReceive(string $request): string;
        public function __construct(string $url = '') {
            $this->url = $url;
            $this->filters = Vector {};
            $this->simple = false;
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
        public function invoke(string $name, Vector<mixed> $arguments = Vector {}, bool $byRef = false, ResultMode $resultMode = ResultMode::Normal, ?bool $simple = null): mixed {
            if ($simple === null) {
                $simple = $this->simple;
            }
            $stream = new StringStream(Tags::TagCall);
            //$hproseWriter = new Writer($stream, $simple);
            //$hproseWriter->writeString($name);
            $stream->write(\hprose_serialize_string($name));
            if (count($arguments) > 0 || $byRef) {
                //$hproseWriter->reset();
                //$hproseWriter->writeList($arguments);
                $stream->write(\hprose_serialize_list($arguments, $simple));
                if ($byRef) {
                    //$hproseWriter->writeBoolean(true);
                    $stream->write(\hprose_serialize_bool(true));
                }
            }
            $stream->write(Tags::TagEnd);
            $request = $stream->toString();
            $count = count($this->filters);
            $context = new \stdClass();
            $context->client = $this;
            $context->userdata = new \stdClass();
            for ($i = 0; $i < $count; $i++) {
                $request = $this->filters[$i]->outputFilter($request, $context);
            }
            $stream->close();
            $response = $this->sendAndReceive($request);
            for ($i = $count - 1; $i >= 0; $i--) {
                $response = $this->filters[$i]->inputFilter($response, $context);
            }
            if ($resultMode == ResultMode::RawWithEndTag) {
                return $response;
            }
            if ($resultMode == ResultMode::Raw) {
                return substr($response, 0, -1);
            }
            $stream = new StringStream($response);
            //$hproseReader = new Reader($stream);
            $hproseReader = new RawReader($stream);
            $result = null;
            while (($tag = $stream->getc()) !== Tags::TagEnd) {
                switch ($tag) {
                    case Tags::TagResult:
                        if ($resultMode == ResultMode::Serialized) {
                            $result = $hproseReader->readRaw()->toString();
                        }
                        else {
                            //$hproseReader->reset();
                            //$result = $hproseReader->unserialize();
                            $result = \hprose_unserialize_with_stream($stream);
                        }
                        break;
                    case Tags::TagArgument:
                        //$hproseReader->reset();
                        //$args = $hproseReader->readList();
                        $args = \hprose_unserialize_with_stream($stream);
                        if ($args instanceof Vector) {
                            for ($i = 0, $n = count($arguments); $i < $n; $i++) {
                                $arguments[$i] = $args[$i];
                            }
                        }
                        break;
                    case Tags::TagError:
                        //$hproseReader->reset();
                        //throw new \Exception($hproseReader->unserialize());
                        throw new \Exception(\hprose_unserialize_string_with_stream($stream));
                        break;
                    default:
                        throw new \Exception("Wrong Response: \r\n" . $response);
                        break;
                }
            }
            return $result;
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
        public function __call(string $name, array<mixed> $arguments): mixed {
            return $this->invoke($name, new Vector($arguments));
        }
        public function __get(string $name): mixed {
            return new Proxy($this, $name . '_');
        }
    }

}

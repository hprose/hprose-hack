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
 * Hprose/Writer.hh                                       *
 *                                                        *
 * hprose writer class for hack.                          *
 *                                                        *
 * LastModified: Feb 26, 2015                             *
 * Author: Ma Bingyao <andot@hprose.com>                  *
 *                                                        *
\**********************************************************/

namespace Hprose {
    interface WriterRefer {
        public function set(mixed $val): void;
        public function write(Stream $stream, mixed $val): bool;
        public function reset(): void;
    }
    class FakeWriterRefer implements WriterRefer {
        public function set(mixed $val): void {}
        public function write(Stream $stream, mixed $val): bool {
            return false;
        }
        public function reset(): void {}
    }
    class RealWriterRefer implements WriterRefer {
        private Vector<mixed> $ref = Vector {};
        private Map<string, int> $sref = Map {};
        private Map<string, int> $bref = Map {};
        private Map<string, int> $oref = Map {};
        private int $refcount = 0;

        private function write_ref(Stream $stream, int $index): bool {
            $stream->write(Tags::TagRef . $index . Tags::TagSemicolon);
            return true;
        }
        public function set(mixed $val): void {
            if (is_string($val)) {
                $this->sref[$val] = $this->refcount;
            }
            elseif ($val instanceof _Bytes) {
                $this->bref[$val->value] = $this->refcount;
            }
            elseif (is_object($val)) {
                $this->ref->add($val);
                $this->oref[spl_object_hash($val)] = $this->refcount;
            }
            $this->refcount++;
        }
        public function write(Stream $stream, mixed $val): bool {
            if (is_string($val) &&
                $this->sref->contains($val)) {
                return $this->write_ref($stream, $this->sref[$val]);
            }
            elseif (($val instanceof _Bytes) &&
                $this->bref->contains($val->value)) {
                return $this->write_ref($stream, $this->bref[$val->value]);
            }
            elseif (is_object($val) &&
                $this->oref->contains(spl_object_hash($val))) {
                return $this->write_ref($stream, $this->oref[spl_object_hash($val)]);
            }
            return false;
        }
        public function reset(): void {
            $this->ref->clear();
            $this->sref->clear();
            $this->bref->clear();
            $this->oref->clear();
            $this->refcount = 0;
        }
    }

    class Writer {
        public Stream $stream;
        private Map<string, int> $classref = Map {};
        private Vector<array<\ReflectionProperty>> $propsref = Vector {};
        private WriterRefer $refer;
        public function __construct(Stream $stream, bool $simple = false) {
            $this->stream = $stream;
            $this->refer = $simple ?
                           new FakeWriterRefer() :
                           new RealWriterRefer();
        }
        public function serialize(mixed $val): void {
            if ($val === null) {
                $this->writeNull();
            }
            elseif (is_scalar($val)) {
                if (is_int($val)) {
                    if ($val >= 0 && $val <= 9) {
                        $this->stream->write((string)$val);
                    }
                    elseif ($val >= -2147483648 && $val <= 2147483647) {
                        $this->writeInteger($val);
                    }
                    else {
                        $this->writeLong((string)$val);
                    }
                }
                elseif (is_bool($val)) {
                    $this->writeBoolean($val);
                }
                elseif (is_float($val)) {
                    $this->writeDouble($val);
                }
                elseif (is_string($val)) {
                    if ($val === '') {
                        $this->writeEmpty();
                    }
                    elseif (strlen($val) < 4 &&
                        is_utf8($val) &&
                        ustrlen($val) == 1) {
                        $this->writeUTF8Char($val);
                    }
                    elseif (is_utf8($val)) {
                        $this->writeStringWithRef($val);
                    }
                    else {
                        $this->writeBytesWithRef(bytes($val));
                    }
                }
            }
            elseif (is_array($val)) {
                if (is_list($val)) {
                    $this->writeArray($val);
                }
                else {
                    $this->writeAssocArray($val);
                }
            }
            elseif (is_object($val)) {
                if ($val instanceof _Bytes) {
                    $this->writeBytesWithRef($val);
                }
                elseif ($val instanceof \DateTime) {
                    $this->writeDateTimeWithRef($val);
                }
                elseif ($val instanceof KeyedTraversable) {
                    $this->writeMapWithRef($val);
                }
                elseif ($val instanceof Traversable) {
                    $this->writeListWithRef($val);
                }
                elseif ($val instanceof _Map) {
                    $this->writeAssocArray($val->value);
                }
                elseif ($val instanceof \stdClass) {
                    $this->writeStdClassWithRef($val);
                }
                else {
                    $this->writeObjectWithRef($val);
                }
            }
            else {
                throw new \Exception('Not support to serialize this data');
            }
        }
        public function writeInteger(int $integer): void {
            $this->stream->write(Tags::TagInteger . $integer . Tags::TagSemicolon);
        }
        public function writeLong(string $long): void {
            $this->stream->write(Tags::TagLong . $long . Tags::TagSemicolon);
        }
        public function writeDouble(float $double): void {
            if (is_nan($double)) {
                $this->writeNaN();
            }
            elseif (is_infinite($double)) {
                $this->writeInfinity($double > 0);
            }
            else {
                $this->stream->write(Tags::TagDouble . $double . Tags::TagSemicolon);
            }
        }
        public function writeNaN(): void {
            $this->stream->write(Tags::TagNaN);
        }
        public function writeInfinity(bool $positive = true): void {
            $this->stream->write(Tags::TagInfinity . ($positive ? Tags::TagPos : Tags::TagNeg));
        }
        public function writeNull(): void {
            $this->stream->write(Tags::TagNull);
        }
        public function writeEmpty(): void {
            $this->stream->write(Tags::TagEmpty);
        }
        public function writeBoolean(bool $bool): void {
            $this->stream->write($bool ? Tags::TagTrue : Tags::TagFalse);
        }
        public function writeUTF8Char(string $char): void {
            $this->stream->write(Tags::TagUTF8Char . $char);
        }
        public function writeString(string $str): void {
            $this->refer->set($str);
            $len = ustrlen($str);
            $this->stream->write(Tags::TagString);
            if ($len > 0) {
                $this->stream->write((string)$len);
            }
            $this->stream->write(Tags::TagQuote . $str . Tags::TagQuote);
        }
        public function writeStringWithRef(string $str): void {
            if (!$this->refer->write($this->stream, $str)) {
                $this->writeString($str);
            }
        }
        public function writeBytes(_Bytes $bytes): void {
            $this->refer->set($bytes);
            $val = $bytes->value;
            $len = strlen($val);
            $this->stream->write(Tags::TagBytes);
            if ($len > 0) {
                $this->stream->write((string)$len);
            }
            $this->stream->write(Tags::TagQuote . $val . Tags::TagQuote);
        }
        public function writeBytesWithRef(_Bytes $bytes): void {
            if (!$this->refer->write($this->stream, $bytes)) {
                $this->writeBytes($bytes);
            }
        }
        public function writeDateTime(\DateTime $datetime): void {
            $this->refer->set($datetime);
            if ($datetime->getOffset() == 0) {
                $this->stream->write($datetime->format('\DYmd\THis.u\Z'));
            }
            else {
                $this->stream->write($datetime->format('\DYmd\THis.u;'));
            }
        }
        public function writeDateTimeWithRef(\DateTime $datetime): void {
            if (!$this->refer->write($this->stream, $datetime)) {
                $this->writeDateTime($datetime);
            }
        }
        public function writeArray(array<mixed> $list): void {
            $this->refer->set($list);
            $count = count($list);
            $this->stream->write(Tags::TagList);
            if ($count > 0) {
                $this->stream->write((string)$count);
            }
            $this->stream->write(Tags::TagOpenbrace);
            foreach ($list as $e) {
                $this->serialize($e);
            }
            $this->stream->write(Tags::TagClosebrace);
        }
        public function writeList(Traversable<mixed> $list): void {
            $this->refer->set($list);
            $count = count($list);
            $this->stream->write(Tags::TagList);
            if ($count > 0) {
                $this->stream->write((string)$count);
            }
            $this->stream->write(Tags::TagOpenbrace);
            foreach ($list as $e) {
                $this->serialize($e);
            }
            $this->stream->write(Tags::TagClosebrace);
        }
        public function writeListWithRef(Traversable<mixed> $list): void {
            if (!$this->refer->write($this->stream, $list)) {
                $this->writeList($list);
            }
        }
        public function writeAssocArray(array<arraykey, mixed> $map): void {
            $this->refer->set($map);
            $count = count($map);
            $this->stream->write(Tags::TagMap);
            if ($count > 0) {
                $this->stream->write((string)$count);
            }
            $this->stream->write(Tags::TagOpenbrace);
            foreach ($map as $key => $value) {
                $this->serialize($key);
                $this->serialize($value);
            }
            $this->stream->write(Tags::TagClosebrace);
        }
        public function writeMap(KeyedTraversable<arraykey, mixed> $map): void {
            $this->refer->set($map);
            $count = count($map);
            $this->stream->write(Tags::TagMap);
            if ($count > 0) {
                $this->stream->write((string)$count);
            }
            $this->stream->write(Tags::TagOpenbrace);
            foreach ($map as $key => $value) {
                $this->serialize($key);
                $this->serialize($value);
            }
            $this->stream->write(Tags::TagClosebrace);
        }
        public function writeMapWithRef(KeyedTraversable<arraykey, mixed> $map): void {
            if (!$this->refer->write($this->stream, $map)) {
                $this->writeMap($map);
            }
        }
        public function writeStdClass(\stdClass $obj): void {
            $this->refer->set($obj);
            $vars = get_object_vars($obj);
            $count = count($vars);
            $this->stream->write(Tags::TagMap);
            if ($count > 0) {
                $this->stream->write((string)$count);
            }
            $this->stream->write(Tags::TagOpenbrace);
            foreach ($vars as $key => $value) {
                $this->serialize($key);
                $this->serialize($value);
            }
            $this->stream->write(Tags::TagClosebrace);
        }
        public function writeStdClassWithRef(\stdClass $obj): void {
            if (!$this->refer->write($this->stream, $obj)) {
                $this->writeStdClass($obj);
            }
        }
        public function writeObject(mixed $obj): void {
            $class = get_class($obj);
            $alias = ClassManager::getClassAlias($class);
            if ($this->classref->contains($alias)) {
                $index = $this->classref[$alias];
            }
            else {
                $reflector = new \ReflectionClass($obj);
                $props = $reflector->getProperties(
                    \ReflectionProperty::IS_PUBLIC |
                    \ReflectionProperty::IS_PROTECTED |
                    \ReflectionProperty::IS_PRIVATE);
                $index = $this->writeClass($alias, $props);
            }
            $this->refer->set($obj);
            $props = $this->propsref[$index];
            $this->stream->write(Tags::TagObject . $index . Tags::TagOpenbrace);
            foreach ($props as $prop) {
                $this->serialize($prop->getValue($obj));
            }
            $this->stream->write(Tags::TagClosebrace);
        }
        public function writeObjectWithRef(mixed $obj): void {
            if (!$this->refer->write($this->stream, $obj)) {
                $this->writeObject($obj);
            }
        }
        protected function writeClass(string $alias, array<\ReflectionProperty> $props): int {
            $len = ustrlen($alias);
            $this->stream->write(Tags::TagClass . $len .
                                 Tags::TagQuote . $alias . Tags::TagQuote);
            $count = count($props);
            if ($count > 0) {
                $this->stream->write((string)$count);
            }
            $this->stream->write(Tags::TagOpenbrace);
            foreach ($props as $prop) {
                $prop->setAccessible(true);
                $this->writeString($prop->getName());
            }
            $this->stream->write(Tags::TagClosebrace);
            $index = count($this->propsref);
            $this->classref[$alias] = $index;
            $this->propsref->add($props);
            return $index;
        }
        public function reset(): void {
            $this->classref->clear();
            $this->propsref->clear();
            $this->refer->reset();
        }
    }
}

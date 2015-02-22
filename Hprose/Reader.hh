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
 * Hprose/Reader.hh                                       *
 *                                                        *
 * hprose reader class for hack.                          *
 *                                                        *
 * LastModified: Feb 22, 2015                             *
 * Author: Ma Bingyao <andot@hprose.com>                  *
 *                                                        *
\**********************************************************/

namespace Hprose {
    require_once('Common.hh');
    require_once('Tags.hh');
    require_once('Stream.hh');
    require_once('ClassManager.hh');

    interface ReaderRefer {
        public function set(mixed $val): void;
        public function read(int $index): mixed;
        public function reset(): void;
    }

    class FakeReaderRefer implements ReaderRefer {
        public function set(mixed $val): void {}
        public function read(int $index): mixed {
            throw new Exception("Unexpected serialize tag '" .
                                Tags::TagRef .
                                "' in stream");
        }
        public function reset(): void {}
    }

    class RealReaderRefer implements ReaderRefer {
        private Vector<mixed> $ref;
        function __construct() {
            $this->reset();
        }
        public function set(mixed $val): void {
            $this->ref->add($val);
        }
        public function read(int $index): mixed {
            return $this->ref[$index];
        }
        public function reset(): void {
            $this->ref = Vector<mixed> {};
        }
    }

    class Reader extends RawReader {
        private Vector<array<string, Vector<string>>> $classref;
        private ReaderRefer $refer;
        function __construct(Stream $stream, bool $simple = false): void {
            parent::__construct($stream);
            $this->classref = Vector<array<string, Vector<string>>> {};
            $this->refer = $simple ? new FakeReaderRefer() : new RealReaderRefer();
        }
        public function unserialize(): mixed {
            $tag = $this->stream->getc();
            $result = NULL;
            switch ($tag) {
                case '0': return 0;
                case '1': return 1;
                case '2': return 2;
                case '3': return 3;
                case '4': return 4;
                case '5': return 5;
                case '6': return 6;
                case '7': return 7;
                case '8': return 8;
                case '9': return 9;
                case Tags::TagInteger: return $this->readIntegerWithoutTag();
                case Tags::TagLong: return $this->readLongWithoutTag();
                case Tags::TagDouble: return $this->readDoubleWithoutTag();
                case Tags::TagNull: return NULL;
                case Tags::TagEmpty: return '';
                case Tags::TagTrue: return true;
                case Tags::TagFalse: return false;
                case Tags::TagNaN: return log(-1);
                case Tags::TagInfinity: return $this->readInfinityWithoutTag();
                case Tags::TagDate: return $this->readDateWithoutTag();
                case Tags::TagTime: return $this->readTimeWithoutTag();
                case Tags::TagBytes: return $this->readBytesWithoutTag();
                case Tags::TagUTF8Char: return $this->readUTF8CharWithoutTag();
                case Tags::TagString: return $this->readStringWithoutTag();
                case Tags::TagGuid: return $this->readGuidWithoutTag();
                case Tags::TagList: return $this->readListWithoutTag();
                case Tags::TagMap: return $this->readMapWithoutTag();
                case Tags::TagClass: $this->readClass(); return $this->readObject();
                case Tags::TagObject: return $this->readObjectWithoutTag();
                case Tags::TagRef: return $this->readRef();
                case Tags::TagError: throw new Exception($this->readString());
                default: return $this->unexpectedTag($tag);
            }
        }
        public function checkTag(string $expectTag, string $tag = NULL): void {
            if (is_null($tag)) {
                $tag = $this->stream->getc();
            }
            if ($tag != $expectTag) {
                $this->unexpectedTag($tag, $expectTag);
            }
        }
        public function checkTags(string $expectTags, string $tag = NULL): string {
            if (is_null($tag)) {
                $tag = $this->stream->getc();
            }
            if (!in_array($tag, $expectTags)) {
                $this->unexpectedTag($tag, implode('', $expectTags));
            }
            return $tag;
        }
        public function readIntegerWithoutTag(): int {
            return (int)($this->stream->readuntil(Tags::TagSemicolon));
        }
        public function readInteger(): int {
            $tag = $this->stream->getc();
            switch ($tag) {
                case '0': return 0;
                case '1': return 1;
                case '2': return 2;
                case '3': return 3;
                case '4': return 4;
                case '5': return 5;
                case '6': return 6;
                case '7': return 7;
                case '8': return 8;
                case '9': return 9;
                case Tags::TagInteger: return $this->readIntegerWithoutTag();
                default: $this->unexpectedTag($tag);
            }
        }
        public function readLongWithoutTag(): string {
            return $this->stream->readuntil(Tags::TagSemicolon);
        }
        public function readLong(): string {
            $tag = $this->stream->getc();
            switch ($tag) {
                case '0': return '0';
                case '1': return '1';
                case '2': return '2';
                case '3': return '3';
                case '4': return '4';
                case '5': return '5';
                case '6': return '6';
                case '7': return '7';
                case '8': return '8';
                case '9': return '9';
                case Tags::TagInteger:
                case Tags::TagLong: return $this->readLongWithoutTag();
                default: $this->unexpectedTag($tag);
            }
        }
        public function readDoubleWithoutTag(): float {
            return (float)($this->stream->readuntil(Tags::TagSemicolon));
        }
        public function readDouble(): float {
            $tag = $this->stream->getc();
            switch ($tag) {
                case '0': return 0;
                case '1': return 1;
                case '2': return 2;
                case '3': return 3;
                case '4': return 4;
                case '5': return 5;
                case '6': return 6;
                case '7': return 7;
                case '8': return 8;
                case '9': return 9;
                case Tags::TagInteger:
                case Tags::TagLong:
                case Tags::TagDouble: return $this->readDoubleWithoutTag();
                case Tags::TagNaN: return log(-1);
                case Tags::TagInfinity: return $this->readInfinityWithoutTag();
                default: $this->unexpectedTag($tag);
            }
        }
        public function readNaN(): float {
            $this->checkTag(Tags::TagNaN);
            return log(-1);
        }
        public function readInfinityWithoutTag(): float {
            return (($this->stream->getc() == Tags::TagNeg) ? log(0) : -log(0));
        }
        public function readInfinity(): float {
            $this->checkTag(Tags::TagInfinity);
            return $this->readInfinityWithoutTag();
        }
        public function readNull(): mixed {
            $this->checkTag(Tags::TagNull);
            return NULL;
        }
        public function readEmpty(): string {
            $this->checkTag(Tags::TagEmpty);
            return '';
        }
        public function readBoolean(): bool {
            $tag = $this->stream->getc();
            switch ($tag) {
                case Tags::TagTrue: return true;
                case Tags::TagFalse: return false;
                default: $this->unexpectedTag($tag);
            }
        }
        public function readDateWithoutTag(): \DateTime {
            $ymd = $this->stream->read(8);
            $hms = '000000';
            $u = '000000';
            $tag = $this->stream->getc();
            if ($tag == Tags::TagTime) {
                $hms = $this->stream->read(6);
                $tag = $this->stream->getc();
                if ($tag == Tags::TagPoint) {
                    $u = $this->stream->read(3);
                    $tag = $this->stream->getc();
                    if (($tag >= '0') && ($tag <= '9')) {
                        $u .= $tag . $this->stream->read(2);
                        $tag = $this->stream->getc();
                        if (($tag >= '0') && ($tag <= '9')) {
                            $this->stream->skip(2);
                            $tag = $this->stream->getc();
                        }
                    }
                    else {
                        $u .= '000';
                    }
                }
            }
            if ($tag == Tags::TagUTC) {
                $date = date_create_from_format('YmdHisu', $ymd.$hms.$u, timezone_open('UTC'));
            }
            else {
                $date = date_create_from_format('YmdHisu', $ymd.$hms.$u);
            }
            $this->refer->set($date);
            return $date;
        }
        public function readDate(): \DateTime {
            $tag = $this->stream->getc();
            switch ($tag) {
                case Tags::TagNull: return NULL;
                case Tags::TagDate: return $this->readDateWithoutTag();
                case Tags::TagRef: return $this->readRef();
                default: $this->unexpectedTag($tag);
            }
        }
        public function readTimeWithoutTag(): \DateTime {
            $hms = $this->stream->read(6);
            $u = '000000';
            $tag = $this->stream->getc();
            if ($tag == Tags::TagPoint) {
                $u = $this->stream->read(3);
                $tag = $this->stream->getc();
                if (($tag >= '0') && ($tag <= '9')) {
                    $u .= $tag . $this->stream->read(2);
                    $tag = $this->stream->getc();
                    if (($tag >= '0') && ($tag <= '9')) {
                        $this->stream->skip(2);
                        $tag = $this->stream->getc();
                    }
                }
                else {
                    $u .= '000';
                }
            }
            if ($tag == Tags::TagUTC) {
                $time = date_create_from_format('Hisu', $hms.$u, timezone_open('UTC'));
            }
            else {
                $time = date_create_from_format('Hisu', $hms.$u);
            }
            $this->refer->set($time);
            return $time;
        }
        public function readTime(): \DateTime {
            $tag = $this->stream->getc();
            switch ($tag) {
                case Tags::TagNull: return NULL;
                case Tags::TagTime: return $this->readTimeWithoutTag();
                case Tags::TagRef: return $this->readRef();
                default: $this->unexpectedTag($tag);
            }
        }
        public function readBytesWithoutTag(): string {
            $count = (int)($this->stream->readuntil(Tags::TagQuote));
            $bytes = $this->stream->read($count);
            $this->stream->skip(1);
            $this->refer->set($bytes);
            return $bytes;
        }
        public function readBytes(): string {
            $tag = $this->stream->getc();
            switch ($tag) {
                case Tags::TagNull: return NULL;
                case Tags::TagEmpty: return '';
                case Tags::TagBytes: return $this->readBytesWithoutTag();
                case Tags::TagRef: return $this->readRef();
                default: $this->unexpectedTag($tag);
            }
        }
        public function readUTF8CharWithoutTag(): string {
            $c = $this->stream->getc();
            $s = $c;
            $a = ord($c);
            if (($a & 0xE0) == 0xC0) {
                $s .= $this->stream->getc();
            }
            elseif (($a & 0xF0) == 0xE0) {
                $s .= $this->stream->read(2);
            }
            elseif ($a > 0x7F) {
                throw new Exception("bad utf-8 encoding");
            }
            return $s;
        }
        public function readUTF8Char(): string {
            $this->checkTag(Tags::TagUTF8Char);
            return $this->readUTF8CharWithoutTag();
        }
        private function _readStringWithoutTag(): string {
            $len = (int)$this->stream->readuntil(Tags::TagQuote);
            $this->stream->mark();
            $utf8len = 0;
            for ($i = 0; $i < $len; ++$i) {
                switch (ord($this->stream->getc()) >> 4) {
                    case 0:
                    case 1:
                    case 2:
                    case 3:
                    case 4:
                    case 5:
                    case 6:
                    case 7: {
                        // 0xxx xxxx
                        $utf8len++;
                        break;
                    }
                    case 12:
                    case 13: {
                        // 110x xxxx   10xx xxxx
                        $this->stream->skip(1);
                        $utf8len += 2;
                        break;
                    }
                    case 14: {
                        // 1110 xxxx  10xx xxxx  10xx xxxx
                        $this->stream->skip(2);
                        $utf8len += 3;
                        break;
                    }
                    case 15: {
                        // 1111 0xxx  10xx xxxx  10xx xxxx  10xx xxxx
                        $this->stream->skip(3);
                        $utf8len += 4;
                        ++$i;
                        break;
                    }
                    default: {
                        throw new Exception('bad utf-8 encoding');
                    }
                }
            }
            $this->stream->reset();
            $this->stream->unmark();
            $s = $this->stream->read($utf8len);
            $this->stream->skip(1);
            return $s;
        }
        public function readStringWithoutTag(): string {
            $s = $this->_readStringWithoutTag();
            $this->refer->set($s);
            return $s;
        }

        public function readString(): string {
            $tag = $this->stream->getc();
            switch ($tag) {
                case Tags::TagNull: return NULL;
                case Tags::TagEmpty: return '';
                case Tags::TagUTF8Char: return $this->readUTF8CharWithoutTag();
                case Tags::TagString: return $this->readStringWithoutTag();
                case Tags::TagRef: return $this->readRef();
                default: $this->unexpectedTag($tag);
            }
        }
        public function readGuidWithoutTag(): string {
            $this->stream->skip(1);
            $s = $this->stream->read(36);
            $this->stream->skip(1);
            $this->refer->set($s);
            return $s;
        }
        public function readGuid(): string {
            $tag = $this->stream->getc();
            switch ($tag) {
                case Tags::TagNull: return NULL;
                case Tags::TagGuid: return $this->readGuidWithoutTag();
                case Tags::TagRef: return $this->readRef();
                default: $this->unexpectedTag($tag);
            }
        }
        public function readListWithoutTag(): Vector {
            $list = Vector {};
            $this->refer->set($list);
            $count = (int)$this->stream->readuntil(Tags::TagOpenbrace);
            for ($i = 0; $i < $count; ++$i) {
                $list->add($this->unserialize());
            }
            $this->stream->skip(1);
            return $list;
        }
        public function readList(): Vector {
            $tag = $this->stream->getc();
            switch ($tag) {
                case Tags::TagNull: return NULL;
                case Tags::TagList: return $this->readListWithoutTag();
                case Tags::TagRef: return $this->readRef();
                default: $this->unexpectedTag($tag);
            }
        }
        public function readMapWithoutTag(): Map {
            $map = Map {};
            $this->refer->set($map);
            $count = (int)$this->stream->readuntil(Tags::TagOpenbrace);
            for ($i = 0; $i < $count; ++$i) {
                $key = $this->unserialize();
                $map[$key] = $this->unserialize();
            }
            $this->stream->skip(1);
            return $map;
        }
        public function readMap(): Map {
            $tag = $this->stream->getc();
            switch ($tag) {
                case Tags::TagNull: return NULL;
                case Tags::TagMap: return $this->readMapWithoutTag();
                case Tags::TagRef: return $this->readRef();
                default: $this->unexpectedTag($tag);
            }
        }
        public function readObjectWithoutTag(): mixed {
            list($classname, $fields) = $this->classref[(int)$this->stream->readuntil(Tags::TagOpenbrace)];
            $object = new $classname;
            $this->refer->set($object);
            $count = count($fields);
            $reflector = new \ReflectionClass($object);
            for ($i = 0; $i < $count; ++$i) {
                $field = $fields[$i];
                if ($reflector->hasProperty($field)) {
                    $property = $reflector->getProperty($field);
                    $property->setAccessible(true);
                    $property->setValue($object, $this->unserialize());
                }
                else {
                    $object->$field = $this->unserialize();
                }
            }
            $this->stream->skip(1);
            return $object;
        }
        public function readObject(): mixed {
            $tag = $this->stream->getc();
            switch ($tag) {
                case Tags::TagNull: return NULL;
                case Tags::TagClass: $this->readclass(); return $this->readObject();
                case Tags::TagObject: return $this->readObjectWithoutTag();
                case Tags::TagRef: return $this->readRef();
                default: $this->unexpectedTag($tag);
            }
        }
        protected function readClass(): void {
            $classname = ClassManager::getClass($this->_readStringWithoutTag());
            $count = (int)$this->stream->readuntil(Tags::TagOpenbrace);
            $fields = Vector<string> {};
            for ($i = 0; $i < $count; ++$i) {
                $fields->add($this->readString());
            }
            $this->stream->skip(1);
            $this->classref->add(array($classname, $fields));
        }
        protected function readRef(): mixed {
            return $this->refer->read((int)$this->stream->readuntil(Tags::TagSemicolon));
        }
        public function reset(): void {
            $this->classref = Vector<array<string, Vector<string>>> {};
            $this->refer->reset();
        }
    }
}

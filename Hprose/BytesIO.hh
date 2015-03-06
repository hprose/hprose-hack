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
 * Hprose/BytesIO.hh                                      *
 *                                                        *
 * hprose BytesIO class for hack                          *
 *                                                        *
 * LastModified: Mar 6, 2015                              *
 * Author: Ma Bingyao <andot@hprose.com>                  *
 *                                                        *
\**********************************************************/

namespace Hprose {
    class BytesIO {
        protected string $buffer;
        protected int $length;
        protected int $pos = 0;
        protected int $mark = -1;
        public function __construct(string $string = '') {
            $this->buffer = $string;
            $this->length = strlen($string);
        }
        public function close(): void {
            $this->buffer = '';
            $this->pos = 0;
            $this->mark = -1;
            $this->length = 0;
        }
        public function length(): int {
            return $this->length;
        }
        public function getc(): string {
            return $this->buffer[$this->pos++];
        }
        public function read(int $n): string {
            $s = substr($this->buffer, $this->pos, $n);
            $this->skip($n);
            return $s;
        }
        public function readfull(): string {
            $s = substr($this->buffer, $this->pos);
            $this->pos = $this->length;
            return $s;
        }
        public function readuntil(string $tag): string {
            $pos = strpos($this->buffer, $tag, $this->pos);
            if ($pos !== false) {
                $s = substr($this->buffer, $this->pos, $pos - $this->pos);
                $this->pos = $pos + strlen($tag);
            }
            else {
                $s = substr($this->buffer, $this->pos);
                $this->pos = $this->length;
            }
            return $s;
        }
        public function readString(int $n): string {
            $pos = $this->pos;
            $buffer = $this->buffer;
            for ($i = 0; $i < $n; ++$i) {
                switch (ord($buffer[$pos]) >> 4) {
                    case 0:
                    case 1:
                    case 2:
                    case 3:
                    case 4:
                    case 5:
                    case 6:
                    case 7: {
                        // 0xxx xxxx
                        ++$pos;
                        break;
                    }
                    case 12:
                    case 13: {
                        // 110x xxxx   10xx xxxx
                        $pos += 2;
                        break;
                    }
                    case 14: {
                        // 1110 xxxx  10xx xxxx  10xx xxxx
                        $pos += 3;
                        break;
                    }
                    case 15: {
                        // 1111 0xxx  10xx xxxx  10xx xxxx  10xx xxxx
                        $pos += 4;
                        ++$i;
                        if ($i >= $n) {
                            throw new \Exception('bad utf-8 encoding');
                        }
                        break;
                    }
                    default: {
                        throw new \Exception('bad utf-8 encoding');
                    }
                }
            }
            return $this->read($pos - $this->pos);
        }
        public function mark(): void {
            $this->mark = $this->pos;
        }
        public function unmark(): void {
            $this->mark = -1;
        }
        public function reset(): void {
            if ($this->mark != -1) {
                $this->pos = $this->mark;
            }
        }
        public function skip(int $n): void {
            $this->pos += $n;
        }
        public function eof(): bool {
            return ($this->pos >= $this->length);
        }
        public function write(string $str, int $n = -1): void {
            if ($n == -1) {
                $this->buffer .= $str;
                $n = strlen($str);
            }
            else {
                $this->buffer .= substr($str, 0, $n);
            }
            $this->length += $n;
        }
        public function toString(): string {
            return $this->buffer;
        }
        public function __toString(): string {
            return $this->buffer;
        }
    }
}

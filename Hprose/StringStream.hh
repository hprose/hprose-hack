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
 * Hprose/StringStream.hh                                 *
 *                                                        *
 * hprose string stream library for hack.                 *
 *                                                        *
 * LastModified: Feb 26, 2015                             *
 * Author: Ma Bingyao <andot@hprose.com>                  *
 *                                                        *
\**********************************************************/

namespace Hprose {
    class StringStream implements Stream {
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
        public function read(int $length): string {
            $s = substr($this->buffer, $this->pos, $length);
            $this->skip($length);
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
        public function write(string $str, int $length = -1): void {
            if ($length == -1) {
                $this->buffer .= $str;
                $length = strlen($str);
            }
            else {
                $this->buffer .= substr($str, 0, $length);
            }
            $this->length += $length;
        }
        public function toString(): string {
            return $this->buffer;
        }
        public function __toString(): string {
            return $this->buffer;
        }
    }
}

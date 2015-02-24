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
 * LastModified: Feb 25, 2015                             *
 * Author: Ma Bingyao <andot@hprose.com>                  *
 *                                                        *
\**********************************************************/

namespace Hprose {
    class StringStream implements Stream {
        protected string $buffer;
        protected int $pos;
        protected int $mark;
        protected int $length;
        public function __construct(string $string = ''): void {
            $this->init($string);
        }
        public function init(string $string): void {
            $this->buffer = $string;
            $this->pos = 0;
            $this->mark = -1;
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
            return $this->buffer{$this->pos++};
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
        public function seek(int $offset, int $whence = SEEK_SET): bool {
            switch ($whence) {
                case SEEK_SET:
                    $this->pos = $offset;
                    break;
                case SEEK_CUR:
                    $this->pos += $offset;
                    break;
                case SEEK_END:
                    $this->pos = $this->length + $offset;
                    break;
            }
            $this->mark = -1;
            return true;
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
                $this->buffer .= substr($string, 0, $length);
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

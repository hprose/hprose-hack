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
 * Hprose/Stream.hh                                       *
 *                                                        *
 * hprose stream interface for hack.                      *
 *                                                        *
 * LastModified: Feb 26, 2015                             *
 * Author: Ma Bingyao <andot@hprose.com>                  *
 *                                                        *
\**********************************************************/

namespace Hprose {
    interface Stream {
        public function close(): void;
        public function length(): int;
        public function getc(): string;
        public function read(int $length): string;
        public function readuntil(string $tag): string;
        public function mark(): void;
        public function unmark(): void;
        public function reset(): void;
        public function skip(int $n): void;
        public function eof(): bool;
        public function write(string $str, int $length = -1): void;
    }
}

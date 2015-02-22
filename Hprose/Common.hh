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
 * Hprose/Common.hh                                       *
 *                                                        *
 * hprose common library for hack.                        *
 *                                                        *
 * LastModified: Feb 18, 2015                             *
 * Author: Ma Bingyao <andot@hprose.com>                  *
 *                                                        *
\**********************************************************/

namespace Hprose {
    class _Bytes {
        public string $value;
        public function __construct(string $val): void {
            $this->value = $val;
        }
        public function __toString(): string {
            return $this->value;
        }
    }

    function bytes(string $val): _Bytes {
        return new _Bytes($val);
    }

    class _Map {
        public array $value;
        public function __construct(array $val): void {
            $this->value = $val;
        }
        public function __toString(): string {
            return var_export($this->value, true);
        }
    }

    function map(array $val): _Map {
        return new _Map($val);
    }

    function is_utf8(string $s): bool {
        return iconv('UTF-8', 'UTF-8//IGNORE', $s) === $s;
    }

    function ustrlen(string $s): int {
        return strlen(iconv('UTF-8', 'UTF-16LE', $s)) >> 1;
    }

    function is_list(array $a): bool {
        $count = count($a);
        return ($count === 0) || ($count === 1 && array_key_exists(0, $a)) || (array_key_exists($count - 1, $a) && array_key_exists(0, $a));
    }
}

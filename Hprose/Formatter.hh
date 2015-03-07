<?hh
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
 * Hprose/Formatter.hh                                    *
 *                                                        *
 * hprose formatter class for hack.                       *
 *                                                        *
 * LastModified: Mar 8, 2015                              *
 * Author: Ma Bingyao <andot@hprose.com>                  *
 *                                                        *
\**********************************************************/

namespace Hprose {
    class Formatter {
        public static function serialize(mixed $var, bool $simple = false): string {
            $stream = new BytesIO();
            $writer = new Writer($stream, $simple);
            $writer->serialize($var);
            return $stream->toString();
        }
        public static function unserialize(string $data, bool $simple = false): mixed {
            $stream = new BytesIO($data);
            $reader = new Reader($stream, $simple);
            return $reader->unserialize();
        }
    }
}

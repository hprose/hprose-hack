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
 * Hprose/Filter.hh                                       *
 *                                                        *
 * hprose filter interface for hack.                      *
 *                                                        *
 * LastModified: Feb 25, 2015                             *
 * Author: Ma Bingyao <andot@hprose.com>                  *
 *                                                        *
\**********************************************************/

namespace Hprose {
    interface Filter {
        public function inputFilter(string $data, \stdClass $context): string;
        public function outputFilter(string $data, \stdClass $context): string;
    }
}

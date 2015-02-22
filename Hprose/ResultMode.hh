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
 * Hprose/ResultMode.hh                                   *
 *                                                        *
 * hprose ResultMode library for hack.                    *
 *                                                        *
 * LastModified: Feb 18, 2015                             *
 * Author: Ma Bingyao <andot@hprose.com>                  *
 *                                                        *
\**********************************************************/

namespace Hprose {
    enum ResultMode: int {
        Normal = 0;
        Serialized = 1;
        Raw = 2;
        RawWithEndTag = 3;
    }
}

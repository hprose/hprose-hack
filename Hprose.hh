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
 * Hprose.hh                                              *
 *                                                        *
 * hprose for hack.                                       *
 *                                                        *
 * LastModified: Feb 18, 2015                             *
 * Author: Ma Bingyao <andot@hprose.com>                  *
 *                                                        *
\**********************************************************/

namespace {
    require_once('Hprose/Common.hh');
    require_once('Hprose/Tags.hh');
    require_once('Hprose/ResultMode.hh');
    require_once('Hprose/StringStream.hh');
    require_once('Hprose/ClassManager.hh');
    require_once('Hprose/Writer.hh');
    require_once('Hprose/RawReader.hh');
    require_once('Hprose/Reader.hh');
    require_once('Hprose/Serialize.hh');
    require_once('Hprose/Unserialize.hh');

    class_alias('Hprose\\_Bytes', 'HproseBytes');
    class_alias('Hprose\\_Map', 'HproseMap');
    class_alias('Hprose\\Tags', 'HproseTags');
    class_alias('Hprose\\ResultMode', 'HproseResultMode');
    class_alias('Hprose\\StringStream', 'HproseStringStream');
    class_alias('Hprose\\ClassManager', 'HproseClassManager');
    class_alias('Hprose\\Writer', 'HproseWriter');
    class_alias('Hprose\\RawReader', 'HproseRawReader');
    class_alias('Hprose\\Reader', 'HproseReader');
}

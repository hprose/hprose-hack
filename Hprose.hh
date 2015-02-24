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
 * LastModified: Feb 24, 2015                             *
 * Author: Ma Bingyao <andot@hprose.com>                  *
 *                                                        *
\**********************************************************/

namespace {
    require('Hprose/Common.hh');
    require('Hprose/Tags.hh');
    require('Hprose/ResultMode.hh');
    require('Hprose/Stream.hh');
    require('Hprose/StringStream.hh');
    require('Hprose/ClassManager.hh');
    require('Hprose/Writer.hh');
    require('Hprose/RawReader.hh');
    require('Hprose/Reader.hh');
    require('Hprose/Serialize.hh');
    require('Hprose/Unserialize.hh');
    require('Hprose/Client.hh');
    require('Hprose/HttpClient.hh');

    class_alias('Hprose\\_Bytes', 'HproseBytes');
    class_alias('Hprose\\_Map', 'HproseMap');
    class_alias('Hprose\\Tags', 'HproseTags');
    class_alias('Hprose\\ResultMode', 'HproseResultMode');
    class_alias('Hprose\\StringStream', 'HproseStringStream');
    class_alias('Hprose\\ClassManager', 'HproseClassManager');
    class_alias('Hprose\\Writer', 'HproseWriter');
    class_alias('Hprose\\RawReader', 'HproseRawReader');
    class_alias('Hprose\\Reader', 'HproseReader');
    class_alias('Hprose\\Client', 'HproseClient');
    class_alias('Hprose\\HttpClient', 'HproseHttpClient');
}

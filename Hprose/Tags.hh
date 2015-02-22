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
 * Hprose/Tags.hh                                         *
 *                                                        *
 * hprose tags library for hack.                          *
 *                                                        *
 * LastModified: Feb 18, 2015                             *
 * Author: Ma Bingyao <andot@hprose.com>                  *
 *                                                        *
\**********************************************************/

namespace Hprose {
    enum Tags: string as string {
        /* Serialize Tags */
        TagInteger = 'i';
        TagLong = 'l';
        TagDouble = 'd';
        TagNull = 'n';
        TagEmpty = 'e';
        TagTrue = 't';
        TagFalse = 'f';
        TagNaN = 'N';
        TagInfinity = 'I';
        TagDate = 'D';
        TagTime = 'T';
        TagUTC = 'Z';
        TagBytes = 'b';
        TagUTF8Char = 'u';
        TagString = 's';
        TagGuid = 'g';
        TagList = 'a';
        TagMap = 'm';
        TagClass = 'c';
        TagObject = 'o';
        TagRef = 'r';
        /* Serialize Marks */
        TagPos = '+';
        TagNeg = '-';
        TagSemicolon = ';';
        TagOpenbrace = '{';
        TagClosebrace = '}';
        TagQuote = '"';
        TagPoint = '.';
        /* Protocol Tags */
        TagFunctions = 'F';
        TagCall = 'C';
        TagResult = 'R';
        TagArgument = 'A';
        TagError = 'E';
        TagEnd = 'z';
    }
}

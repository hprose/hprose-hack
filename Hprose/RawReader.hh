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
 * Hprose/RawReader.hh                                    *
 *                                                        *
 * hprose raw reader class for hack.                      *
 *                                                        *
 * LastModified: Feb 22, 2015                             *
 * Author: Ma Bingyao <andot@hprose.com>                  *
 *                                                        *
\**********************************************************/

namespace Hprose {
    require_once('Common.hh');
    require_once('Tags.hh');
    require_once('Stream.hh');
    require_once('StringStream.hh');

    class RawReader {
        public Stream $stream;
        function __construct(Stream $stream): void {
            $this->stream = $stream;
        }
        public function unexpectedTag(string $tag, string $expectTags = ''): void {
            if ($tag && $expectTags) {
                throw new Exception("Tag '" . $expectTags . "' expected, but '" . $tag . "' found in stream");
            }
            else if ($tag) {
                throw new Exception("Unexpected serialize tag '" . $tag . "' in stream");
            }
            else {
                throw new Exception('No byte found in stream');
            }
        }
        public function readRaw(Stream $ostream = NULL, string $tag = NULL): Stream {
            if ($ostream === NULL) {
                $ostream = new StringStream();
            }
            if ($tag === NULL) {
                $tag = $this->stream->getc();
            }
            $ostream->write($tag);
            switch ($tag) {
                case '0':
                case '1':
                case '2':
                case '3':
                case '4':
                case '5':
                case '6':
                case '7':
                case '8':
                case '9':
                case Tags::TagNull:
                case Tags::TagEmpty:
                case Tags::TagTrue:
                case Tags::TagFalse:
                case Tags::TagNaN:
                    break;
                case Tags::TagInfinity:
                    $ostream->write($this->stream->getc());
                    break;
                case Tags::TagInteger:
                case Tags::TagLong:
                case Tags::TagDouble:
                case Tags::TagRef:
                    $this->readNumberRaw($ostream);
                    break;
                case Tags::TagDate:
                case Tags::TagTime:
                    $this->readDateTimeRaw($ostream);
                    break;
                case Tags::TagUTF8Char:
                    $this->readUTF8CharRaw($ostream);
                    break;
                case Tags::TagBytes:
                    $this->readBytesRaw($ostream);
                    break;
                case Tags::TagString:
                    $this->readStringRaw($ostream);
                    break;
                case Tags::TagGuid:
                    $this->readGuidRaw($ostream);
                    break;
                case Tags::TagList:
                case Tags::TagMap:
                case Tags::TagObject:
                    $this->readComplexRaw($ostream);
                    break;
                case Tags::TagClass:
                    $this->readComplexRaw($ostream);
                    $this->readRaw($ostream);
                    break;
                case Tags::TagError:
                    $this->readRaw($ostream);
                    break;
                default: $this->unexpectedTag($tag);
            }
            return $ostream;
        }

        private function readNumberRaw(Stream $ostream): void {
            $s = $this->stream->readuntil(Tags::TagSemicolon) .
                 Tags::TagSemicolon;
            $ostream->write($s);
        }

        private function readDateTimeRaw(Stream $ostream): void {
            $s = '';
            do {
                $tag = $this->stream->getc();
                $s .= $tag;
            } while ($tag != Tags::TagSemicolon &&
                     $tag != Tags::TagUTC);
            $ostream->write($s);
        }

        private function readUTF8CharRaw(Stream $ostream): void {
            $tag = $this->stream->getc();
            $s = $tag;
            $a = ord($tag);
            if (($a & 0xE0) == 0xC0) {
                $s .= $this->stream->getc();
            }
            elseif (($a & 0xF0) == 0xE0) {
                $s .= $this->stream->read(2);
            }
            elseif ($a > 0x7F) {
                throw new Exception("bad utf-8 encoding");
            }
            $ostream->write($s);
        }

        private function readBytesRaw(Stream $ostream): void {
            $len = $this->stream->readuntil(Tags::TagQuote);
            $s = $len . Tags::TagQuote . $this->stream->read((int)$len) . Tags::TagQuote;
            $this->stream->skip(1);
            $ostream->write($s);
        }

        private function readStringRaw(Stream $ostream): void {
            $len = $this->stream->readuntil(Tags::TagQuote);
            $s = $len . Tags::TagQuote;
            $len = (int)$len;
            $this->stream->mark();
            $utf8len = 0;
            for ($i = 0; $i < $len; ++$i) {
                switch (ord($this->stream->getc()) >> 4) {
                    case 0:
                    case 1:
                    case 2:
                    case 3:
                    case 4:
                    case 5:
                    case 6:
                    case 7: {
                        // 0xxx xxxx
                        $utf8len++;
                        break;
                    }
                    case 12:
                    case 13: {
                        // 110x xxxx   10xx xxxx
                        $this->stream->skip(1);
                        $utf8len += 2;
                        break;
                    }
                    case 14: {
                        // 1110 xxxx  10xx xxxx  10xx xxxx
                        $this->stream->skip(2);
                        $utf8len += 3;
                        break;
                    }
                    case 15: {
                        // 1111 0xxx  10xx xxxx  10xx xxxx  10xx xxxx
                        $this->stream->skip(3);
                        $utf8len += 4;
                        ++$i;
                        break;
                    }
                    default: {
                        throw new Exception('bad utf-8 encoding');
                    }
                }
            }
            $this->stream->reset();
            $this->stream->unmark();
            $s .= $this->stream->read($utf8len) . Tags::TagQuote;
            $this->stream->skip(1);
            $ostream->write($s);
        }

        private function readGuidRaw(Stream $ostream): void {
            $ostream->write($this->stream->read(38));
        }

        private function readComplexRaw(Stream $ostream): void {
            $s = $this->stream->readuntil(Tags::TagOpenbrace) .
                 Tags::TagOpenbrace;
            $ostream->write($s);
            while (($tag = $this->stream->getc()) != Tags::TagClosebrace) {
                $this->readRaw($ostream, $tag);
            }
            $ostream->write($tag);
        }
    }
}

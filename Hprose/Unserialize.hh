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
 * Hprose/Unserialize.hh                                  *
 *                                                        *
 * hprose unserialize library for hack.                   *
 *                                                        *
 * LastModified: Feb 26, 2015                             *
 * Author: Ma Bingyao <andot@hprose.com>                  *
 *                                                        *
\**********************************************************/

namespace Hprose {
    // private functions

    /* $t is a 1 byte character. */
    function readuntil(\stdClass $o, string $t): string {
        $p = strpos($o->s, $t, $o->p);
        if ($p != false) {
            $r = substr($o->s, $o->p, $p - $o->p);
            $o->p = $p + 1;
        }
        else {
            $r = substr($o->s, $o->p);
            $o->p = strlen($o->s);
        }
        return $r;
    }

    function read_ref(\stdClass $o): mixed {
        return $o->r[(int)readuntil($o, ';')];
    }

    function read_utf8char(\stdClass $o): string {
        $c = $o->s[$o->p++];
        switch (ord($c) >> 4) {
            case 0:
            case 1:
            case 2:
            case 3:
            case 4:
            case 5:
            case 6:
            case 7: return $c;
            case 12:
            case 13: return $c . $o->s[$o->p++];
            case 14: return $c . $o->s[$o->p++] . $o->s[$o->p++];
        }
        throw new \Exception('bad utf-8 encoding');
    }

    function read_string(\stdClass $o): string {
        $l = (int)readuntil($o, '"');
        $p = $o->p;
        for ($i = 0; $i < $l; ++$i) {
            switch (ord($o->s[$o->p]) >> 4) {
                case 0:
                case 1:
                case 2:
                case 3:
                case 4:
                case 5:
                case 6:
                case 7: ++$o->p; break;
                case 12:
                case 13: $o->p += 2; break;
                case 14: $o->p += 3; break;
                case 15: $o->p += 4; ++$i; break;
                default: throw new \Exception('bad utf-8 encoding');
            }
        }
        $s = substr($o->s, $p, $o->p - $p);
        ++$o->p;
        return $s;
    }

    function simple_unserialize_string(\stdClass $o): string {
        switch ($o->s[$o->p++]) {
            case 'u': return read_utf8char($o);
            case 's': return read_string($o);
            case 'E': throw new \Exception(simple_unserialize_string($o));
        }
        throw new \Exception("Can't unserialize '$o->s' as string.");
    }

    function fast_unserialize_string(\stdClass $o): string {
        switch ($o->s[$o->p++]) {
            case 'u': return read_utf8char($o);
            case 's': return $o->r[] = read_string($o);
            case 'r': return (string)read_ref($o);
            case 'E': throw new \Exception(fast_unserialize_string($o));
        }
        throw new \Exception("Can't unserialize '$o->s' as string.");
    }

    function read_bytes(\stdClass $o): string {
        $c = (int)readuntil($o, '"');
        $bytes = substr($o->s, $o->p, $c);
        $o->p += $c + 1;
        return $bytes;
    }

    function read_guid(\stdClass $o): string {
        $g = substr($o->s, $o->p + 1, 36);
        $o->p += 38;
        return $g;
    }

    function __read_time(\stdClass $o): (string, string) {
        $hms = substr($o->s, $o->p, 6);
        $o->p += 6;
        $u = '000000';
        $tag = $o->s[$o->p++];
        if ($tag == '.') {
            $u = substr($o->s, $o->p, 3);
            $o->p += 3;
            $tag = $o->s[$o->p++];
            if (($tag >= '0') && ($tag < '9')) {
                $u .= $tag . substr($o->s, $o->p, 2);
                $o->p += 2;
                $tag = $o->s[$o->p++];
                if (($tag >= '0') && ($tag < '9')) {
                    $o->p += 2;
                    $tag = $o->s[$o->p++];
                }
            }
            else {
                $u .= '000';
            }
        }
        return tuple($hms.$u, $tag);
    }

    function read_date(\stdClass $o): \DateTime {
        $ymd = substr($o->s, $o->p, 8);
        $hmsu = '000000000000';
        $o->p += 8;
        $tag = $o->s[$o->p++];
        if ($tag == 'T') {
            list($hmsu, $tag) = __read_time($o);
        }
        if ($tag == 'Z') {
            $date = date_create_from_format('YmdHisu', $ymd.$hmsu, timezone_open('UTC'));
        }
        else {
            $date = date_create_from_format('YmdHisu', $ymd.$hmsu);
        }
        return $date;
    }

    function read_time(\stdClass $o): \DateTime {
        list($hmsu, $tag) = __read_time($o);
        if ($tag == 'Z') {
            $date = date_create_from_format('Hisu', $hmsu, timezone_open('UTC'));
        }
        else {
            $date = date_create_from_format('Hisu', $hmsu);
        }
        return $date;
    }

    function simple_read_list(\stdClass $o): Vector<mixed> {
        $a = Vector {};
        $c = (int)readuntil($o, '{');
        for ($i = 0; $i < $c; ++$i) {
            $a->add(simple_unserialize($o));
        }
        ++$o->p;
        return $a;
    }

    function fast_read_list(\stdClass $o): Vector<mixed> {
        $a = Vector {};
        $o->r->add($a);
        $c = (int)readuntil($o, '{');
        for ($i = 0; $i < $c; ++$i) {
            $a->add(fast_unserialize($o));
        }
        ++$o->p;
        return $a;
    }

    function simple_read_map(\stdClass $o): Map<arraykey, mixed> {
        $m = Map {};
        $c = (int)readuntil($o, '{');
        for ($i = 0; $i < $c; ++$i) {
            $k = simple_unserialize_key($o);
            $m[$k] = simple_unserialize($o);
        }
        ++$o->p;
        return $m;
    }

    function fast_read_map(\stdClass $o): Map<arraykey, mixed> {
        $m = Map {};
        $o->r->add($m);
        $c = (int)readuntil($o, '{');
        for ($i = 0; $i < $c; ++$i) {
            $k = fast_unserialize_key($o);
            $m[$k] = fast_unserialize($o);
        }
        ++$o->p;
        return $m;
    }

    function simple_read_class(\stdClass $o): void {
        $classname = ClassManager::getClass(read_string($o));
        $c = (int)readuntil($o, '{');
        $props = Vector {};
        for ($i = 0; $i < $c; ++$i) {
            $props->add(simple_unserialize_string($o));
        }
        ++$o->p;
        $o->cr->add(tuple($classname, $props));
    }

    function fast_read_class(\stdClass $o): void {
        $classname = ClassManager::getClass(read_string($o));
        $c = (int)readuntil($o, '{');
        $props = Vector {};
        for ($i = 0; $i < $c; ++$i) {
            $props->add(fast_unserialize_string($o));
        }
        ++$o->p;
        $o->cr->add(tuple($classname, $props));
    }

    function simple_read_object(\stdClass $o): mixed {
        list($classname, $props) = $o->cr[(int)readuntil($o, '{')];
        if ($classname == 'stdClass') {
            $obj = new \stdClass();
            foreach ($props as $prop) {
                // UNSAFE
                $obj->$prop = simple_unserialize($o);
            }
        }
        else {
            $reflector = new \ReflectionClass($classname);
            if ($reflector->getConstructor() === null) {
                $obj = $reflector->newInstanceWithoutConstructor();
            }
            else {
                $obj = $reflector->newInstance();
            }
            foreach ($props as $prop) {
                if ($reflector->hasProperty($prop)) {
                    $property = $reflector->getProperty($prop);
                    $property->setAccessible(true);
                    $property->setValue($obj, simple_unserialize($o));
                }
            }
        }
        ++$o->p;
        return $obj;
    }

    function fast_read_object(\stdClass $o): mixed {
        list($classname, $props) = $o->cr[(int)readuntil($o, '{')];
        if ($classname == 'stdClass') {
            $obj = new \stdClass();
            $o->r->add($obj);
            foreach ($props as $prop) {
                // UNSAFE
                $obj->$prop = fast_unserialize($o);
            }
        }
        else {
            $reflector = new \ReflectionClass($classname);
            if ($reflector->getConstructor() === null) {
                $obj = $reflector->newInstanceWithoutConstructor();
            }
            else {
                $obj = $reflector->newInstance();
            }
            $o->r->add($obj);
            foreach ($props as $prop) {
                if ($reflector->hasProperty($prop)) {
                    $property = $reflector->getProperty($prop);
                    $property->setAccessible(true);
                    $property->setValue($obj, fast_unserialize($o));
                }
            }
        }
        ++$o->p;
        return $obj;
    }

    function simple_unserialize_key(\stdClass $o): arraykey {
        switch ($o->s[$o->p++]) {
            case '0': return 0;
            case '1': return 1;
            case '2': return 2;
            case '3': return 3;
            case '4': return 4;
            case '5': return 5;
            case '6': return 6;
            case '7': return 7;
            case '8': return 8;
            case '9': return 9;
            case 'n': return 'null';
            case 'e': return '';
            case 't': return 'true';
            case 'f': return 'false';
            case 'N': return (string)log(-1);
            case 'I': return (string)($o->s[$o->p++] == '-' ? log(0) : -log(0));
            case 'i': return (int)readuntil($o, ';');
            case 'l': return readuntil($o, ';');
            case 'd': return readuntil($o, ';');
            case 'u': return read_utf8char($o);
            case 's': return read_string($o);
            case 'b': return read_bytes($o);
            case 'g': return read_guid($o);
            case 'E': throw new \Exception(simple_unserialize_string($o));
        }
        throw new \Exception("Can't unserialize '{$o->s}' in simple mode.");
    }

    function simple_unserialize(\stdClass $o): mixed {
        switch ($o->s[$o->p++]) {
            case '0': return 0;
            case '1': return 1;
            case '2': return 2;
            case '3': return 3;
            case '4': return 4;
            case '5': return 5;
            case '6': return 6;
            case '7': return 7;
            case '8': return 8;
            case '9': return 9;
            case 'n': return null;
            case 'e': return '';
            case 't': return true;
            case 'f': return false;
            case 'N': return log(-1);
            case 'I': return ($o->s[$o->p++] == '-' ? log(0) : -log(0));
            case 'i': return (int)readuntil($o, ';');
            case 'l': return readuntil($o, ';');
            case 'd': return (float)readuntil($o, ';');
            case 'u': return read_utf8char($o);
            case 's': return read_string($o);
            case 'b': return read_bytes($o);
            case 'g': return read_guid($o);
            case 'D': return read_date($o);
            case 'T': return read_time($o);
            case 'a': return simple_read_list($o);
            case 'm': return simple_read_map($o);
            case 'c': simple_read_class($o); return simple_unserialize($o);
            case 'o': return simple_read_object($o);
            case 'E': throw new \Exception(simple_unserialize_string($o));
        }
        throw new \Exception("Can't unserialize '{$o->s}' in simple mode.");
    }

    function fast_unserialize_key(\stdClass $o): arraykey {
        switch ($o->s[$o->p++]) {
            case '0': return 0;
            case '1': return 1;
            case '2': return 2;
            case '3': return 3;
            case '4': return 4;
            case '5': return 5;
            case '6': return 6;
            case '7': return 7;
            case '8': return 8;
            case '9': return 9;
            case 'n': return 'null';
            case 'e': return '';
            case 't': return 'true';
            case 'f': return 'false';
            case 'N': return (string)log(-1);
            case 'I': return (string)($o->s[$o->p++] == '-' ? log(0) : -log(0));
            case 'i': return (int)readuntil($o, ';');
            case 'l': return readuntil($o, ';');
            case 'd': return readuntil($o, ';');
            case 'u': return read_utf8char($o);
            case 's': return $o->r[] = read_string($o);
            case 'b': return $o->r[] = read_bytes($o);
            case 'g': return $o->r[] = read_guid($o);
            case 'r': return (string)read_ref($o);
            case 'E': throw new \Exception(fast_unserialize_string($o));
        }
        throw new \Exception("Can't unserialize '{$o->s}'.");
    }

    function fast_unserialize(\stdClass $o): mixed {
        switch ($o->s[$o->p++]) {
            case '0': return 0;
            case '1': return 1;
            case '2': return 2;
            case '3': return 3;
            case '4': return 4;
            case '5': return 5;
            case '6': return 6;
            case '7': return 7;
            case '8': return 8;
            case '9': return 9;
            case 'n': return null;
            case 'e': return '';
            case 't': return true;
            case 'f': return false;
            case 'N': return log(-1);
            case 'I': return ($o->s[$o->p++] == '-' ? log(0) : -log(0));
            case 'i': return (int)readuntil($o, ';');
            case 'l': return readuntil($o, ';');
            case 'd': return (float)readuntil($o, ';');
            case 'u': return read_utf8char($o);
            case 's': return $o->r[] = read_string($o);
            case 'b': return $o->r[] = read_bytes($o);
            case 'g': return $o->r[] = read_guid($o);
            case 'D': return $o->r[] = read_date($o);
            case 'T': return $o->r[] = read_time($o);
            case 'a': return fast_read_list($o);
            case 'm': return fast_read_map($o);
            case 'c': fast_read_class($o); return fast_unserialize($o);
            case 'o': return fast_read_object($o);
            case 'r': return read_ref($o);
            case 'E': throw new \Exception(fast_unserialize_string($o));
        }
        throw new \Exception("Can't unserialize '{$o->s}'.");
    }
}


namespace {
    // public functions
    function hprose_unserialize(string $s, bool $simple = false): mixed {
        $o = new stdClass();
        $o->s = $s;
        $o->p = 0;
        $o->cr = Vector {};
        if ($simple) {
            $v = Hprose\simple_unserialize($o);
        }
        else {
            $o->r = Vector {};
            $v = Hprose\fast_unserialize($o);
        }
        return $v;
    }

    function hprose_unserialize_with_stream(Hprose\Stream $s, bool $simple = false): mixed {
        $o = new stdClass();
        $s->mark();
        $o->s = $s->readfull();
        $o->p = 0;
        $o->cr = Vector {};
        if ($simple) {
            $v = Hprose\simple_unserialize($o);
        }
        else {
            $o->r = Vector {};
            $v = Hprose\fast_unserialize($o);
        }
        $s->reset();
        $s->skip($o->p);
        return $v;
    }

    function hprose_unserialize_string_with_stream(Hprose\Stream $s, bool $simple = false): string {
        $o = new stdClass();
        $s->mark();
        $o->s = $s->readfull();
        $o->p = 0;
        if ($simple) {
            $v = Hprose\simple_unserialize_string($o);
        }
        else {
            $o->r = Vector {};
            $v = Hprose\fast_unserialize_string($o);
        }
        $s->reset();
        $s->skip($o->p);
        return $v;
    }

    function hprose_unserialize_list_with_stream(Hprose\Stream $s): Vector<mixed> {
        $o = new stdClass();
        $s->mark();
        $o->s = $s->readfull();
        $o->p = 0;
        $o->cr = Vector {};
        $o->r = Vector {};
        $v = Hprose\fast_read_list($o);
        $s->reset();
        $s->skip($o->p);
        return $v;
    }

/*
    function hprose_unserialize(string $s, bool $simple = false): mixed {
        $reader = new Hprose\Reader(new Hprose\StringStream($s), $simple);
        return $reader->unserialize();
    }
    function hprose_unserialize_with_stream(StringStream $s, bool $simple = false): mixed {
        $reader = new Hprose\Reader($s, $simple);
        return $reader->unserialize();
    }

    function hprose_unserialize_list_with_stream(StringStream $s): Vector {
        $reader = new Hprose\Reader($s);
        return $reader->readList();
    }
*/

}

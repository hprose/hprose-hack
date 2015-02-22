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
 * Hprose/Serialize.hh                                    *
 *                                                        *
 * hprose serialize library for hack.                     *
 *                                                        *
 * LastModified: Feb 22, 2015                             *
 * Author: Ma Bingyao <andot@hprose.com>                  *
 *                                                        *
\**********************************************************/

namespace Hprose {
    require_once('Common.hh');
    require_once('ClassManager.hh');

    // private functions

    function simple_serialize(mixed $v, \stdClass $ro): string {
        if ($v === NULL) {
            return 'n';
        }
        elseif (is_int($v)) {
            if ($v >= 0 && $v <= 9) {
                return (string)$v;
            }
            elseif ($v >= -2147483648 && $v <= 2147483647) {
                return 'i' . $v . ';';
            }
            else {
                return 'l' . $v . ';';
            }
        }
        elseif (is_bool($v)) {
            return $v ? 't' : 'f';
        }
        elseif (is_float($v)) {
            if (is_nan($v)) {
                return 'N';
            }
            elseif (is_infinite($v)) {
                return $v > 0 ? 'I+' : 'I-';
            }
            else {
                return 'd' . $v . ';';
            }
        }
        elseif (is_string($v)) {
            if ($v === '') {
                return 'e';
            }
            elseif (is_utf8($v)) {
                $l = ustrlen($v);
                if ($l == 1) {
                    return 'u' . $v;
                }
                else {
                    return 's' . $l . '"' . $v . '"';
                }
            }
            else {
                return 'b' . strlen($v) . '"' . $v . '"';
            }
        }
        elseif (is_array($v)) {
            $c = count($v);
            if ($c == 0) {
                return 'a{}';
            }
            if (is_list($v)) {
                $s = 'a' . $c . '{';
                foreach ($v as $val) {
                    $s .= simple_serialize($val, $ro);
                }
            }
            else {
                $s = 'm' . $c . '{';
                foreach ($v as $key => $val) {
                    $s .= simple_serialize($key, $ro) .
                          simple_serialize($val, $ro);
                }
            }
            return $s . '}';
        }
        elseif (is_object($v)) {
            if ($v instanceof _Bytes) {
                $c = strlen($v->value);
                if ($c == 0) {
                    return 'b""';
                }
                return 'b' . $c . '"' . $v->value . '"';
            }
            elseif ($v instanceof _Map) {
                $c = count($v->value);
                if ($c == 0) {
                    return 'm{}';
                }
                $s = 'm' . $c . '{';
                foreach ($v->value as $key => $val) {
                    $s .= simple_serialize($key, $ro) .
                          simple_serialize($val, $ro);
                }
                return $s . '}';
            }
            elseif ($v instanceof \DateTime) {
                if ($v->getOffset() == 0) {
                    return $v->format("\\DYmd\\THis.u\\Z");
                }
                return $v->format("\\DYmd\\THis.u;");
            }
            elseif ($v instanceof \stdClass) {
                $v = get_object_vars($v);
                $c = count($v);
                if ($c == 0) {
                    return 'a{}';
                }
                $s = 'm' . $c . '{';
                foreach ($v as $key => $val) {
                    $s .= simple_serialize($key, $ro) .
                          simple_serialize($val, $ro);
                }
                return $s . '}';
            }
            elseif ($v instanceof Vector || $v instanceof Set) {
                $c = count($v);
                if ($c == 0) {
                    return 'a{}';
                }
                $s = 'a' . $c . '{';
                foreach ($v as $val) {
                    $s .= simple_serialize($val, $ro);
                }
                return $s . '}';
            }
            elseif ($v instanceof Map) {
                $c = count($v);
                if ($c == 0) {
                    return 'm{}';
                }
                $s = 'm' . $c . '{';
                foreach ($v as $key => $val) {
                    $s .= simple_serialize($key, $ro) .
                          simple_serialize($val, $ro);
                }
                return $s . '}';
            }
            else {
                $class = get_class($v);
                $alias = ClassManager::getClassAlias($class);
                $a = (array)$v;
                if ($ro->cr->contains($alias)) {
                    $index = $ro->cr[$alias];
                    $fields = $ro->fr[$index];
                    $c = count($fields);
                    $s = '';
                }
                else {
                    $s = 'c' . ustrlen($alias) . '"' . $alias . '"';
                    $fields = array_keys($a);
                    $c = count($fields);
                    if ($c > 0) {
                        $s .= $c . '{';
                        foreach ($fields as $field) {
                            if ($field[0] === "\0") {
                                $field = substr($field, strpos($field, "\0", 1) + 1);
                            }
                            $s .= 's' . ustrlen($field) . '"' . $field . '"';
                        }
                        $s .= '}';
                    }
                    else {
                        $s .= '{}';
                    }
                    $index = count($ro->fr);
                    $ro->cr[$alias] = $index;
                    $ro->fr->add($fields);
                }
                $s .= 'o' . $index . '{';
                for ($i = 0; $i < $c; ++$i) {
                    $s .= simple_serialize($a[$fields[$i]], $ro);
                }
                return $s . '}';
            }
        }
        else {
            throw new Exception('Not support to serialize this data');
        }
    }

    function fast_serialize(mixed $v, \stdClass $ro): string {
        if ($v === NULL) {
            return 'n';
        }
        elseif (is_int($v)) {
            if ($v >= 0 && $v <= 9) {
                return (string)$v;
            }
            elseif ($v >= -2147483648 && $v <= 2147483647) {
                return 'i' . $v . ';';
            }
            else {
                return 'l' . $v . ';';
            }
        }
        elseif (is_bool($v)) {
            return $v ? 't' : 'f';
        }
        elseif (is_float($v)) {
            if (is_nan($v)) {
                return 'N';
            }
            elseif (is_infinite($v)) {
                return $v > 0 ? 'I+' : 'I-';
            }
            else {
                return 'd' . $v . ';';
            }
        }
        elseif (is_string($v)) {
            if ($v === '') {
                return 'e';
            }
            if (is_utf8($v)) {
                $l = ustrlen($v);
                if ($l == 1) {
                    return 'u' . $v;
                }
                if ($ro->sr->contains($v)) {
                    return 'r' . $ro->sr[$v] . ';';
                }
                else {
                    $ro->sr[$v] = $ro->count++;
                    return 's' . $l . '"' . $v . '"';
                }
            }
            else {
                if ($ro->br->contains($v)) {
                    return 'r' . $ro->br[$v] . ';';
                }
                else {
                    $ro->br[$v] = $ro->count++;
                    return 'b' . strlen($v) . '"' . $v . '"';
                }
            }
        }
        if (is_array($v)) {
            $c = count($v);
            $ro->count++;
            if ($c == 0) {
                return 'a{}';
            }
            if (is_list($v)) {
                $s = 'a' . $c . '{';
                foreach ($v as $val) {
                    $s .= fast_serialize($val, $ro);
                }
            }
            else {
                $s = 'm' . $c . '{';
                foreach ($v as $key => $val) {
                    $s .= fast_serialize($key, $ro) .
                          fast_serialize($val, $ro);
                }
            }
            return $s . '}';
        }
        if (is_object($v)) {
            if ($v instanceof _Bytes) {
                $v = $v->value;
                if ($ro->br->contains($v)) {
                    return 'r' . $ro->br[$v] . ';';
                }
                else {
                    $ro->br[$v] = $ro->count++;
                    return 'b' . strlen($v) . '"' . $v . '"';
                }
            }
            elseif ($v instanceof _Map) {
                $ro->count++;
                $c = count($v->value);
                if ($c == 0) {
                    return 'm{}';
                }
                $s = 'm' . $c . '{';
                foreach ($v->value as $key => $val) {
                    $s .= fast_serialize($key, $ro) .
                          fast_serialize($val, $ro);
                }
                return $s . '}';
            }
            else {
                $h = spl_object_hash($v);
                if ($ro->or->contains($h)) {
                    return 'r' . $ro->or[$h] . ';';
                }
                $ro->r->add($v);
                if ($v instanceof \DateTime) {
                    $ro->or[$h] = $ro->count++;
                    if ($v->getOffset() == 0) {
                        return $v->format("\\DYmd\\THis.u\\Z");
                    }
                    return $v->format("\\DYmd\\THis.u;");
                }
                elseif ($v instanceof \stdClass) {
                    $ro->or[$h] = $ro->count++;
                    $v = get_object_vars($v);
                    $c = count($v);
                    if ($c == 0) {
                        return 'a{}';
                    }
                    $s = 'm' . $c . '{';
                    foreach ($v as $key => $val) {
                        $s .= fast_serialize($key, $ro) .
                              fast_serialize($val, $ro);
                    }
                    return $s . '}';
                }
                elseif ($v instanceof Vector || $v instanceof Set) {
                    $ro->or[$h] = $ro->count++;
                    $c = count($v);
                    if ($c == 0) {
                        return 'a{}';
                    }
                    $s = 'a' . $c . '{';
                    foreach ($v as $val) {
                        $s .= fast_serialize($val, $ro);
                    }
                    return $s . '}';
                }
                elseif ($v instanceof Map) {
                    $ro->or[$h] = $ro->count++;
                    $c = count($v);
                    if ($c == 0) {
                        return 'm{}';
                    }
                    $s = 'm' . $c . '{';
                    foreach ($v as $key => $val) {
                        $s .= fast_serialize($key, $ro) .
                              fast_serialize($val, $ro);
                    }
                    return $s . '}';
                }
                else {
                    $class = get_class($v);
                    $alias = ClassManager::getClassAlias($class);
                    $a = (array)$v;
                    if ($ro->cr->contains($alias)) {
                        $index = $ro->cr[$alias];
                        $fields = $ro->fr[$index];
                        $c = count($fields);
                        $s = '';
                    }
                    else {
                        $s = 'c' . ustrlen($alias) . '"' . $alias . '"';
                        $fields = array_keys($a);
                        $c = count($fields);
                        if ($c > 0) {
                            $s .= $c . '{';
                            foreach ($fields as $field) {
                                if ($field[0] === "\0") {
                                    $field = substr($field, strpos($field, "\0", 1) + 1);
                                }
                                $ro->sr[$field] = $ro->count++;
                                $s .= 's' . ustrlen($field) . '"' . $field . '"';
                            }
                            $s .= '}';
                        }
                        else {
                            $s .= '{}';
                        }
                        $index = count($ro->fr);
                        $ro->cr[$alias] = $index;
                        $ro->fr->add($fields);
                    }
                    $ro->or[$h] = $ro->count++;
                    $s .= 'o' . $index . '{';
                    for ($i = 0; $i < $c; ++$i) {
                        $s .= fast_serialize($a[$fields[$i]], $ro);
                    }
                    return $s . '}';
                }
            }
        }
        throw new Exception('Not support to serialize this data');
    }
}

namespace {
    // public functions
    function hprose_serialize_bool(bool $b): string {
        return $b ? 't' : 'f';
    }

    function hprose_serialize_string(string $s): string {
        return 's' . ustrlen($s) . '"' . $s . '"';
    }

    function hprose_serialize_list(mixed $a, bool $simple = false): string {
        $c = count($a);
        if ($c == 0) return 'a{}';
        $ro = new stdClass();
        $ro->cr = Map<string, int> {};
        $ro->fr = Vector<array<string>> {};
        if ($simple) {
            $s = 'a' . $c . '{';
            foreach ($a as $v) {
                $s .= Hprose\simple_serialize($v, $ro);
            }
            return $s . '}';
        }
        $ro->sr = Map<string, int> {};
        $ro->br = Map<string, int> {};
        $ro->or = Map<string, int> {};
        $ro->r = Vector<mixed> {};
        $ro->count = 1;
        $s = 'a' . $c . '{';
        foreach ($a as $v) {
            $s .= Hprose\fast_serialize($v, $ro);
        }
        return $s . '}';
    }
    function hprose_serialize(mixed $v, bool $simple = false): string {
        $ro = new stdClass();
        $ro->cr = Map<string, int> {};
        $ro->fr = Vector<array<string>> {};
        if ($simple) {
            return Hprose\simple_serialize($v, $ro);
        }
        $ro->sr = Map<string, int> {};
        $ro->br = Map<string, int> {};
        $ro->or = Map<string, int> {};
        $ro->r = Vector<mixed> {};
        $ro->count = 0;
        return Hprose\fast_serialize($v, $ro);
    }
/*
    function hprose_serialize(mixed $v, bool $simple = false): string {
        $writer = new Hprose\Writer(new Hprose\StringStream(), $simple);
        $writer->serialize($v);
        return $writer->stream->toString();
    }
*/
}

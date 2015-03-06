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
 * LastModified: Mar 6, 2015                              *
 * Author: Ma Bingyao <andot@hprose.com>                  *
 *                                                        *
\**********************************************************/

namespace Hprose {
    // private functions
    function is_utf8(string $s): bool {
        return iconv('UTF-8', 'UTF-8//IGNORE', $s) === $s;
    }

    function ustrlen(string $s): int {
        return strlen(iconv('UTF-8', 'UTF-16LE', $s)) >> 1;
    }

    function is_list(array<arraykey, mixed> $a): bool {
        $count = count($a);
        // UNSAFE
        return ($count === 0) ||
               ($count === 1 && (isset($a[0]) || array_key_exists(0, $a))) ||
               ((isset($a[$count - 1]) || array_key_exists($count - 1, $a)) &&
                (isset($a[0]) || array_key_exists(0, $a)));
    }
    function simple_serialize(mixed $v, \stdClass $ro): string {
        if ($v === null) {
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
            if ($v instanceof \DateTime) {
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
            elseif ($v instanceof KeyedTraversable) {
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
            elseif ($v instanceof Traversable) {
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
            else {
                $class = get_class($v);
                $alias = ClassManager::getClassAlias($class);
                if ($ro->cr->contains($alias)) {
                    $index = $ro->cr[$alias];
                    $props = $ro->pr[$index];
                    $s = '';
                }
                else {
                    $s = 'c' . ustrlen($alias) . '"' . $alias . '"';
                    $reflector = new \ReflectionClass($v);
                    $props = $reflector->getProperties(
                        \ReflectionProperty::IS_PUBLIC |
                        \ReflectionProperty::IS_PROTECTED |
                        \ReflectionProperty::IS_PRIVATE);
                    $c = count($props);
                    if ($c > 0) {
                        $s .= $c . '{';
                        foreach ($props as $prop) {
                            $prop->setAccessible(true);
                            $name = $prop->getName();
                            $s .= 's' . ustrlen($name) . '"' . $name . '"';
                        }
                        $s .= '}';
                    }
                    else {
                        $s .= '{}';
                    }
                    $index = count($ro->pr);
                    $ro->cr[$alias] = $index;
                    $ro->pr->add($props);
                }
                $s .= 'o' . $index . '{';
                foreach ($props as $prop) {
                    $s .= simple_serialize($prop->getValue($v), $ro);
                }
                return $s . '}';
            }
        }
        else {
            throw new \Exception('Not support to serialize this data');
        }
    }

    function fast_serialize(mixed $v, \stdClass $ro): string {
        if ($v === null) {
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
            elseif ($v instanceof KeyedTraversable) {
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
            elseif ($v instanceof Traversable) {
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
            else {
                $class = get_class($v);
                $alias = ClassManager::getClassAlias($class);
                if ($ro->cr->contains($alias)) {
                    $index = $ro->cr[$alias];
                    $props = $ro->pr[$index];
                    $s = '';
                }
                else {
                    $s = 'c' . ustrlen($alias) . '"' . $alias . '"';
                    $reflector = new \ReflectionClass($v);
                    $props = $reflector->getProperties(
                        \ReflectionProperty::IS_PUBLIC |
                        \ReflectionProperty::IS_PROTECTED |
                        \ReflectionProperty::IS_PRIVATE);
                    $c = count($props);
                    if ($c > 0) {
                        $s .= $c . '{';
                        foreach ($props as $prop) {
                            $prop->setAccessible(true);
                            $name = $prop->getName();
                            $ro->sr[$name] = $ro->count++;
                            $s .= 's' . ustrlen($name) . '"' . $name . '"';
                        }
                        $s .= '}';
                    }
                    else {
                        $s .= '{}';
                    }
                    $index = count($ro->pr);
                    $ro->cr[$alias] = $index;
                    $ro->pr->add($props);
                }
                $ro->or[$h] = $ro->count++;
                $s .= 'o' . $index . '{';
                foreach ($props as $prop) {
                    $s .= fast_serialize($prop->getValue($v), $ro);
                }
                return $s . '}';
            }
        }
        throw new \Exception('Not support to serialize this data');
    }
}

namespace {
    // public functions
    function hprose_serialize_bool(bool $b): string {
        return $b ? 't' : 'f';
    }

    function hprose_serialize_string(string $s): string {
        return 's' . Hprose\ustrlen($s) . '"' . $s . '"';
    }

    function hprose_serialize_list(Traversable<mixed> $a, bool $simple = false): string {
        $c = count($a);
        if ($c == 0) return 'a{}';
        $ro = new stdClass();
        $ro->cr = Map {};
        $ro->pr = Vector {};
        if ($simple) {
            $s = 'a' . $c . '{';
            foreach ($a as $v) {
                $s .= Hprose\simple_serialize($v, $ro);
            }
            return $s . '}';
        }
        $ro->sr = Map {};
        $ro->br = Map {};
        $ro->or = Map {};
        $ro->r = Vector {};
        $ro->count = 1;
        $s = 'a' . $c . '{';
        foreach ($a as $v) {
            $s .= Hprose\fast_serialize($v, $ro);
        }
        return $s . '}';
    }
    function hprose_serialize(mixed $v, bool $simple = false): string {
        $ro = new stdClass();
        $ro->cr = Map {};
        $ro->pr = Vector {};
        if ($simple) {
            return Hprose\simple_serialize($v, $ro);
        }
        $ro->sr = Map {};
        $ro->br = Map {};
        $ro->or = Map {};
        $ro->r = Vector {};
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

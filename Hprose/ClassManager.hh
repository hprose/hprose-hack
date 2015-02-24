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
 * Hprose/ClassManager.hh                                 *
 *                                                        *
 * hprose class manager library for hack.                 *
 *                                                        *
 * LastModified: Feb 24, 2015                             *
 * Author: Ma Bingyao <andot@hprose.com>                  *
 *                                                        *
\**********************************************************/

namespace Hprose {
    class ClassManager {
        private static Map<string, string> $classCache1 = Map {};
        private static Map<string, string> $classCache2 = Map {};
        public static function register(string $class, string $alias): void {
            self::$classCache1[$alias] = $class;
            self::$classCache2[$class] = $alias;
        }
        public static function getClassAlias(string $class): string {
            if (self::$classCache2->contains($class)) {
                return self::$classCache2[$class];
            }
            $alias = str_replace('\\', '_', $class);
            self::register($class, $alias);
            return $alias;
        }
        public static function getClass(string $alias): string {
            if (self::$classCache1->contains($alias)) {
                return self::$classCache1[$alias];
            }
            if (!class_exists($alias)) {
                $class = str_replace('_', '\\', $alias);
                if (class_exists($class)) {
                    self::register($class, $alias);
                    return $class;
                }
                return 'stdClass';
            }
            return $alias;
        }
    }
}

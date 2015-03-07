<?hh
namespace Hprose {
    include("../Hprose.hh");

    function runtime($mode = 0) {
        static $s;
        if (!$mode) {
            $s = microtime();
            return;
        }
        $e = microtime();
        $s = explode(" ", $s);
        $e = explode(" ", $e);
        return sprintf("%.2f ms\r\n", ($e[1]+$e[0]-$s[1]-$s[0])*1000);
    }

    date_default_timezone_set('Asia/Shanghai');

    class User {
        public function __construct(
            public string $name = '',
            public int $age = 0,
            public ?User $self = null) {}
    }

    ClassManager::register('Hprose\\User', 'User');

    $ss = new BytesIO();
    $writer = new Writer($ss);
    $writer->serialize(NULL);
    $writer->serialize(1);
    $writer->serialize(0);
    $writer->serialize(9);
    $writer->serialize(10);
    $writer->serialize(-2147483648);
    $writer->serialize(2147483648);
    $writer->serialize(2147483647);
    $writer->serialize(3.1415926);
    $writer->serialize(NAN);
    $writer->serialize(INF);
    $writer->serialize(-INF);
    $writer->serialize(true);
    $writer->serialize(false);
    $writer->serialize(\DateTime::createFromFormat('YmdHis.u', '20150219143448.123456'));
    $writer->serialize(\DateTime::createFromFormat('YmdHis.u', '20150219143448.123456', new \DateTimeZone('UTC')));
    $writer->serialize([1,2,3,4,5]);
    $writer->serialize(tuple(1,2,3,4,5));
    $writer->serialize(Vector {1,2,3,4,5});
    $writer->serialize(Set {1,2,3,4,5});
    $writer->serialize(["name" => "张三", "age" => 32]);
    $m = Map {"name" => "张三", "age" => 32};
    $writer->serialize($m);
    $writer->serialize($m);
    $o = new \stdClass();
    $o->name = "李四";
    $o->age = 18;
    $user = new User();
    $user->name = "李四";
    $user->age = 18;
    $user->self = $user;
    $writer->serialize($o);
    $writer->serialize($user);

    var_dump($ss);

    $reader = new Reader($ss);

    var_dump($reader->unserialize());
    var_dump($reader->unserialize());
    var_dump($reader->unserialize());
    var_dump($reader->unserialize());
    var_dump($reader->unserialize());
    var_dump($reader->unserialize());
    var_dump($reader->unserialize());
    var_dump($reader->unserialize());
    var_dump($reader->unserialize());
    var_dump($reader->unserialize());
    var_dump($reader->unserialize());
    var_dump($reader->unserialize());
    var_dump($reader->unserialize());
    var_dump($reader->unserialize());
    var_dump($reader->unserialize());
    var_dump($reader->unserialize());
    var_dump($reader->unserialize());
    var_dump($reader->unserialize());
    var_dump($reader->unserialize());
    var_dump($reader->unserialize());
    var_dump($reader->unserialize());
    var_dump($reader->unserialize());
    var_dump($reader->unserialize());
    var_dump($reader->unserialize());
    var_dump($reader->unserialize());

    var_dump(hprose_serialize(NULL));
    var_dump(hprose_serialize(1));
    var_dump(hprose_serialize(0));
    var_dump(hprose_serialize(9));
    var_dump(hprose_serialize(10));
    var_dump(hprose_serialize(-2147483648));
    var_dump(hprose_serialize(2147483648));
    var_dump(hprose_serialize(2147483647));
    var_dump(hprose_serialize(3.1415926));
    var_dump(hprose_serialize(NAN));
    var_dump(hprose_serialize(INF));
    var_dump(hprose_serialize(-INF));
    var_dump(hprose_serialize(true));
    var_dump(hprose_serialize(false));
    var_dump(hprose_serialize(\DateTime::createFromFormat('YmdHis.u', '20150219143448.123456')));
    var_dump(hprose_serialize(\DateTime::createFromFormat('YmdHis.u', '20150219143448.123456', new \DateTimeZone('UTC'))));
    var_dump(hprose_serialize([1,2,3,4,5]));
    var_dump(hprose_serialize(tuple(1,2,3,4,5)));
    var_dump(hprose_serialize(Vector {1,2,3,4,5}));
    var_dump(hprose_serialize(Set {1,2,3,4,5}));
    var_dump(hprose_serialize(["name" => "张三", "age" => 32]));
    var_dump(hprose_serialize($m));
    var_dump(hprose_serialize($o));
    var_dump(hprose_serialize($user));

    var_dump(hprose_unserialize(hprose_serialize(NULL)));
    var_dump(hprose_unserialize(hprose_serialize(1)));
    var_dump(hprose_unserialize(hprose_serialize(0)));
    var_dump(hprose_unserialize(hprose_serialize(9)));
    var_dump(hprose_unserialize(hprose_serialize(10)));
    var_dump(hprose_unserialize(hprose_serialize(-2147483648)));
    var_dump(hprose_unserialize(hprose_serialize(2147483648)));
    var_dump(hprose_unserialize(hprose_serialize(2147483647)));
    var_dump(hprose_unserialize(hprose_serialize(3.1415926)));
    var_dump(hprose_unserialize(hprose_serialize(NAN)));
    var_dump(hprose_unserialize(hprose_serialize(INF)));
    var_dump(hprose_unserialize(hprose_serialize(-INF)));
    var_dump(hprose_unserialize(hprose_serialize(true)));
    var_dump(hprose_unserialize(hprose_serialize(false)));
    var_dump(hprose_unserialize(hprose_serialize(\DateTime::createFromFormat('YmdHis.u', '20150219143448.123456'))));
    var_dump(hprose_unserialize(hprose_serialize(\DateTime::createFromFormat('YmdHis.u', '20150219143448.123456', new \DateTimeZone('UTC')))));
    var_dump(hprose_unserialize(hprose_serialize([1,2,3,4,5])));
    var_dump(hprose_unserialize(hprose_serialize(tuple(1,2,3,4,5))));
    var_dump(hprose_unserialize(hprose_serialize(Vector {1,2,3,4,5})));
    var_dump(hprose_unserialize(hprose_serialize(Set {1,2,3,4,5})));
    var_dump(hprose_unserialize(hprose_serialize(["name" => "张三", "age" => 32])));
    var_dump(hprose_unserialize(hprose_serialize($m)));
    var_dump(hprose_unserialize(hprose_serialize($o)));
    var_dump(hprose_unserialize(hprose_serialize($user)));

    runtime();
    for ($i = 0; $i < 1000; $i++) {
        hprose_unserialize(hprose_serialize(NULL));
        hprose_unserialize(hprose_serialize(1));
        hprose_unserialize(hprose_serialize(0));
        hprose_unserialize(hprose_serialize(9));
        hprose_unserialize(hprose_serialize(10));
        hprose_unserialize(hprose_serialize(-2147483648));
        hprose_unserialize(hprose_serialize(2147483648));
        hprose_unserialize(hprose_serialize(2147483647));
        hprose_unserialize(hprose_serialize(3.1415926));
        hprose_unserialize(hprose_serialize(NAN));
        hprose_unserialize(hprose_serialize(INF));
        hprose_unserialize(hprose_serialize(-INF));
        hprose_unserialize(hprose_serialize(true));
        hprose_unserialize(hprose_serialize(false));
        hprose_unserialize(hprose_serialize(\DateTime::createFromFormat('YmdHis.u', '20150219143448.123456')));
        hprose_unserialize(hprose_serialize(\DateTime::createFromFormat('YmdHis.u', '20150219143448.123456', new \DateTimeZone('UTC'))));
        hprose_unserialize(hprose_serialize([1,2,3,4,5]));
        hprose_unserialize(hprose_serialize(tuple(1,2,3,4,5)));
        hprose_unserialize(hprose_serialize(Vector {1,2,3,4,5}));
        hprose_unserialize(hprose_serialize(Set {1,2,3,4,5}));
        hprose_unserialize(hprose_serialize(["name" => "张三", "age" => 32]));
        hprose_unserialize(hprose_serialize($m));
        hprose_unserialize(hprose_serialize($o));
        hprose_unserialize(hprose_serialize($user));
    }
    print(runtime(1));

    runtime();
    for ($i = 0; $i < 1000; $i++) {
        unserialize(serialize(NULL));
        unserialize(serialize(1));
        unserialize(serialize(0));
        unserialize(serialize(9));
        unserialize(serialize(10));
        unserialize(serialize(-2147483648));
        unserialize(serialize(2147483648));
        unserialize(serialize(2147483647));
        unserialize(serialize(3.1415926));
        unserialize(serialize(NAN));
        unserialize(serialize(INF));
        unserialize(serialize(-INF));
        unserialize(serialize(true));
        unserialize(serialize(false));
        unserialize(serialize(\DateTime::createFromFormat('YmdHis.u', '20150219143448.123456')));
        unserialize(serialize(\DateTime::createFromFormat('YmdHis.u', '20150219143448.123456', new \DateTimeZone('UTC'))));
        unserialize(serialize([1,2,3,4,5]));
        unserialize(serialize(tuple(1,2,3,4,5)));
        unserialize(serialize(Vector {1,2,3,4,5}));
        unserialize(serialize(Set {1,2,3,4,5}));
        unserialize(serialize(["name" => "张三", "age" => 32]));
        unserialize(serialize($m));
        unserialize(serialize($o));
        unserialize(serialize($user));
    }
    print(runtime(1));

    function test(int $a, string $s, $aa)  {
        return array($s => $a);
    }

    var_dump(null == '');

    $test = new \ReflectionFunction("\\Hprose\\test");
    $params = $test->getParameters();
    foreach ($params as $param) {
        var_dump($param->getTypeText());
    }

    $client = new HttpClient('http://127.0.0.1/server.hh');
    var_dump($client->hello("World"));

    $client->hello("async hello", function($result, $args, $error) {
        echo "result: ";
        var_dump($result);
        echo "args: ";
        var_dump($args);
        echo "error: ";
        var_dump($error);
    });
    $client->hello("async hello2", function($result, $args, $error) {
        echo "result: ";
        var_dump($result);
        echo "args: ";
        var_dump($args);
        echo "error: ";
        var_dump($error);
    });
    $client->loop();
    $client->hello("async hello3", function($result, $args, $error) {
        echo "result: ";
        var_dump($result);
        echo "args: ";
        var_dump($args);
        echo "error: ";
        var_dump($error);
    });
    $client->hello("async hello4", function($result, $args, $error) {
        echo "result: ";
        var_dump($result);
        echo "args: ";
        var_dump($args);
        echo "error: ";
        var_dump($error);
    });
    $client->hello("async hello5", function($result, $args, $error) {
        echo "result: ";
        var_dump($result);
        echo "args: ";
        var_dump($args);
        echo "error: ";
        var_dump($error);
    });
}

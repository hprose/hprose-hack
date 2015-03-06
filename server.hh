<?hh

include("Hprose.hh");

function hello(string $name): string {
    return "Hello " . $name;
}

$server = new HproseHttpServer();
$server->addFunction(fun("hello"));
$server->start();

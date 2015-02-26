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
 * Hprose/HttpService.hh                                  *
 *                                                        *
 * hprose http service library for hack.                  *
 *                                                        *
 * LastModified: Feb 27, 2015                             *
 * Author: Ma Bingyao <andot@hprose.com>                  *
 *                                                        *
\**********************************************************/

namespace Hprose {
    class HttpService extends Service {
        private bool $crossDomain = false;
        private bool $P3P = false;
        private bool $get = true;
        private Map<string, bool> $origins = Map {};
        public ?(function(\stdClass): void) $onSendHeader = null;

        private function sendHeader(\stdClass $context): void {
            if ($this->onSendHeader !== null) {
                $sendHeader = $this->onSendHeader;
                $sendHeader($context);
            }
            header("Content-Type: text/plain");
            if ($this->P3P) {
                header('P3P: CP="CAO DSP COR CUR ADM DEV TAI PSA PSD ' .
                       'IVAi IVDi CONi TELo OTPi OUR DELi SAMi OTRi ' .
                       'UNRi PUBi IND PHY ONL UNI PUR FIN COM NAV ' .
                       'INT DEM CNT STA POL HEA PRE GOV"');
            }
            if ($this->crossDomain) {
                // UNSAFE
                if (array_key_exists('HTTP_ORIGIN', $_SERVER) &&
                    $_SERVER['HTTP_ORIGIN'] != "null") {
                    $origin = $_SERVER['HTTP_ORIGIN'];
                    if ($this->origins->contains($origin)) {
                        header("Access-Control-Allow-Origin: " . $origin);
                        header("Access-Control-Allow-Credentials: true");
                    }
                }
                else {
                    header('Access-Control-Allow-Origin: *');
                }
            }
        }
        public function isCrossDomainEnabled(): bool {
            return $this->crossDomain;
        }
        public function setCrossDomainEnabled(bool $enable = true): void {
            $this->crossDomain = $enable;
        }
        public function isP3PEnabled(): bool {
            return $this->P3P;
        }
        public function setP3PEnabled(bool $enable = true): void {
            $this->P3P = $enable;
        }
        public function isGetEnabled(): bool {
            return $this->get;
        }
        public function setGetEnabled(bool $enable = true): void {
            $this->get = $enable;
        }
        public function addAccessControlAllowOrigin(string $origin): void {
            $this->origins[$origin] = true;
        }
        public function removeAccessControlAllowOrigin(string $origin): void {
            $this->origins->remove($origin);
        }
        public function handle(): void {
            $request = file_get_contents("php://input");

            $context = new \stdClass();
            $context->server = $this;
            $context->userdata = new \stdClass();

            set_error_handler(($errno, $errstr, $errfile, $errline) ==> {
                if ($this->debug) {
                    $errstr .= " in $errfile on line $errline";
                }
                $error = self::$errorTable[$errno] . ": " . $errstr;
                exit($this->sendError($error, $context));
            }, $this->error_types);

            ob_start($data ==> {
                $match = array();
                if (preg_match('/<b>.*? error<\/b>:(.*?)<br/', $data, $match)) {
                    if ($this->debug) {
                        $error = preg_replace('/<.*?>/', '', $match[1]);
                    }
                    else {
                        $error = preg_replace('/ in <b>.*<\/b>$/', '', $match[1]);
                    }
                    return $this->sendError(trim($error), $context);
                }
                return $this->outputFilter($data, $context);
            });

            ob_implicit_flush(0);
            @ob_clean();
            $this->sendHeader($context);
            $result = '';
            // UNSAFE
            if (($_SERVER['REQUEST_METHOD'] == 'GET') && $this->get) {
                $result = $this->doFunctionList($context);
            }
            elseif ($_SERVER['REQUEST_METHOD'] == 'POST') {
                $result = $this->defaultHandle($request, $context);
            }
            @ob_end_clean();
            exit($result);
        }
    }
}

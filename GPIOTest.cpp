#include <iostream>
#include <thread>

#include "src/GPIO.h"
#include "src/Tool.h"

#define DO_TEST "/sys/class/leds/do6/brightness"
#define DI_TEST "/dev/POW_DET"

class GPIOManager : public gpio::GPIO {
private:
    GPIOManager() {
    }
    ~GPIOManager() {
    }

protected:
    // 读到数据之后
    virtual void onRead(const std::string &portName, const char *buf, size_t len, const std::string &error) {
        if (!error.empty()) {
            std::cout << "GPIO错误数据,portName:" << portName << ",error:" << error << std::endl;
            return;
        }
        int value = atoi(std::string(buf, len).data());
        std::cout << "GPIO收到数据,portName:" << portName << ",data:" << value << ",len:" << len << std::endl;
        {
            std::lock_guard<std::mutex> lock(mutex_);
            DIStatus_[portName] = value;
        }
    }
    // 数据写入之后
    virtual void onWrite(const std::string &portName, const int len, const std::string &error) {
        if (!error.empty()) {
            std::cout << "onWrite,error:" << error << std::endl;
        }
    }
    // 连接上之后
    virtual void onConnect(const std::string &portName, const std::string &error) {
        if (!error.empty()) {
            std::cout << "onConnect,error:" << error << std::endl;
        }
    }
    // 连接关闭之前
    virtual void onClose(const std::string &portName, const std::string &error) {
        if (!error.empty()) {
            std::cout << "onClose,error:" << error << std::endl;
        }
    }
    // 定时器发生之后
    virtual void onTimer(const std::string &portName) {
    }
    // 监听错误发生之后
    void onListenError(const std::string &portName, const std::string &error) {
        if (!error.empty()) {
            std::cout << "onListenError,error:" << error << std::endl;
            return;
        }
    }

public:
    static GPIOManager &instance() {
        static GPIOManager ins;
        return ins;
    }
    void start() {
        std::string err;
        if (!add(err, DO_TEST, gpio::writeOnly)) {
            std::cout << "添加gpio失败,portName:" << DO_TEST << ",err:" << err << std::endl;
        }

        if (!add(err, DI_TEST, gpio::readOnly)) {
            std::cout << "添加gpio失败,portName:" << DI_TEST << ",err:" << err << std::endl;
        }
    }

public:
    bool set(const std::string &name, bool flag) {
        std::string value = "0";
        if (flag) {
            value = "1";
        }
        return send(name, value);
    }
    int get(const std::string &name) {
        std::lock_guard<std::mutex> lock(mutex_);
        if (DIStatus_.find(name) == DIStatus_.end()) {
            return -1;
        }
        return DIStatus_[name];
    }

private:
    std::mutex mutex_;
    std::map<std::string, int> DIStatus_;
};

int main(int argc, char *argv[]) {
    GPIOManager::instance().start();
    while (1) {
        char ch = getchar();
        if (ch == 'q') {
            exit(0);
        } else if (ch == '1') {
            std::cout << "DO测试,打开DO" << std::endl;
            GPIOManager::instance().set(DO_TEST, true);
        } else if (ch == '2') {
            std::cout << "DO测试,关闭DO" << std::endl;
            GPIOManager::instance().set(DO_TEST, false);
        } else if (ch == '3') {
            std::cout << "DI测试,当前DI状态为:" << GPIOManager::instance().get(DI_TEST) << std::endl;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }
}

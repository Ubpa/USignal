#include <USignal/USignal.hpp>

using namespace Ubpa;

class Counter {
public:
    Counter() { m_value = 0; }

    int value() const { return m_value; }

    Signal<int> valueChanged;
    void setValue(int value) {
        if (value != m_value) {
            m_value = value;
            valueChanged.Emit(value);
        }
    }

private:
    int m_value;
};

int main() {
    Counter a, b;
    a.valueChanged.Connect<&Counter::setValue>(&b);
    a.setValue(12); // a.value() == 12, b.value() == 12
    b.setValue(48); // a.value() == 12, b.value() == 48

    return 0;
}
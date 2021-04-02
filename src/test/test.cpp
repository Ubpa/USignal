#include <gtest/gtest.h>

#include <USignal/USignal.hpp>

using namespace Ubpa;

TEST(Signal, ctor) {
	Signal<int, float> sig;
}

TEST(Signal, basic) {
	Signal<int, float> sig;
	constexpr int arg_0 = 1;
	constexpr float arg_1 = 2.f;
	bool called = false;
	sig.Connect([&called](int a, float b) {
		EXPECT_EQ(a, arg_0);
		EXPECT_EQ(b, arg_1);
		called = true;
	});
	sig.Emit(arg_0, arg_1);
	EXPECT_TRUE(called);
}

TEST(Signal, compatible) {
	Signal<int, float> sig;
	constexpr int arg_0 = 1;
	constexpr float arg_1 = 2.f;
	sig.Connect([](const int& a, const float& b) {
		EXPECT_EQ(a, arg_0);
		EXPECT_EQ(b, arg_1);
	});
	sig.Connect([](const int&& a) {
		EXPECT_EQ(a, arg_0);
	});
	sig.Emit(arg_0, arg_1);
}

TEST(Signal, zero_arg) {
	Signal<> sig;
	bool called = false;
	sig.Connect([&called] {called = true; });
	sig.Emit();
	EXPECT_TRUE(called);
}

TEST(Signal, return) {
	Signal<> sig;
	bool called = false;
	sig.Connect([&called] () -> int {
		called = true;
		return 1;
	});
	sig.Emit();
	EXPECT_TRUE(called);
}

TEST(Signal, disconnct) {
	Signal<int, float> sig;
	bool called = false;
	Connection connection = sig.Connect([&called](int, float) -> int {
		called = true;
		return 1;
	});
	sig.Disconnect(std::move(connection));
	sig.Emit(0,0.f);
	EXPECT_FALSE(called);
}

TEST(Signal, clear) {
	Signal<int, float> sig;
	bool called_0 = false;
	bool called_1 = false;
	sig.Connect([&called_0](int, float) -> int {
		called_0 = true;
		return 1;
	});
	sig.Connect([&called_1](int, float) -> int {
		called_1 = true;
		return 1;
	});
	sig.Clear();
	sig.Emit(0, 0.f);
	EXPECT_FALSE(called_0);
	EXPECT_FALSE(called_1);
}

TEST(Signal, memfunc) {
	Signal<int, float> sig;
	struct C {
		void slot_0() { called_0 = true; }
		void slot_1() { called_1 = true; }
		void slot_overload() { called_2 = true; }
		void slot_overload(int, float) { called_3 = true; }
		bool called_0 = false;
		bool called_1 = false;
		bool called_2 = false;
		bool called_3 = false;
	};
	C c;
	sig.Connect<&C::slot_0>(&c);
	sig.Connect(&c, &C::slot_1);
	sig.Connect<void()>(&c, &C::slot_overload);
	sig.Connect<MemFuncOf<C, void(int, float)>::get(&C::slot_overload)>(&c);

	sig.Emit(0, 0.f);
	EXPECT_TRUE(c.called_0);
	EXPECT_TRUE(c.called_1);
	EXPECT_TRUE(c.called_2);
	EXPECT_TRUE(c.called_3);
}

TEST(Signal, callable_obj) {
	Signal<int, float> sig;
	struct Func {
		bool called = false;
		void operator()(int, float) {
			called = true;
		}
	};
	Func f;
	sig.Connect(&f);
	sig.Emit(0, 0.f);
	EXPECT_TRUE(f.called);
}

TEST(Signal, disconnect_obj) {
	Signal<int, float> sig;
	struct Func {
		int cnt_call_op = 0;
		int cnt_call_f = 0;
		void operator()(int, float) {
			cnt_call_op++;
		}
		void f(){
			cnt_call_f++;
		}
	};
	Func f;
	sig.Connect(&f);
	sig.Connect<&Func::f>(&f);
	sig.Emit(0, 0.f);
	sig.Emit(0, 0.f);
	sig.Disconnect(&f);
	sig.Emit(0, 0.f);
	EXPECT_EQ(f.cnt_call_op, 2);
	EXPECT_EQ(f.cnt_call_f, 2);
}

TEST(Signal, disconnect_pro) {
	Signal<int, float> sig;
	struct Func {
		int cnt_call_op = 0;
		int cnt_call_f = 0;
		void operator()(int, float) {
			cnt_call_op++;
		}
		void f() {
			cnt_call_f++;
		}
	};
	Func f;
	sig.Connect(&f);
	sig.Connect<&Func::f>(&f);
	sig.Emit(0, 0.f);
	sig.Disconnect<&Func::f>(&f);
	sig.Emit(0, 0.f);
	sig.Disconnect<&Func::operator()>(&f);
	sig.Emit(0, 0.f);
	EXPECT_EQ(f.cnt_call_op, 2);
	EXPECT_EQ(f.cnt_call_f, 1);
}

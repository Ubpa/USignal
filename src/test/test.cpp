#include <gtest/gtest.h>

#include <USignal/USignal.hpp>

using namespace Ubpa;

TEST(Signal, ctor) {
	Signal<void(int, float)> sig;
}

TEST(Signal, basic) {
	Signal<void(int, float)> sig;
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
	Signal<void(int, float)> sig;
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
	Signal<void()> sig;
	bool called = false;
	sig.Connect([&called] {called = true; });
	sig.Emit();
	EXPECT_TRUE(called);
}

TEST(Signal, return) {
	Signal<void()> sig;
	bool called = false;
	sig.Connect([&called] () -> int {
		called = true;
		return 1;
	});
	sig.Emit();
	EXPECT_TRUE(called);
}

TEST(Signal, disconnct) {
	Signal<void(int, float)> sig;
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
	Signal<void(int, float)> sig;
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
	Signal<void(int, float)> sig;
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
	sig.Connect(&C::slot_1, &c);
	sig.Connect(MemFuncOf<C, void()>::get(&C::slot_overload), &c);
	sig.Connect<MemFuncOf<C, void(int, float)>::get(&C::slot_overload)>(&c);

	sig.Emit(0, 0.f);
	EXPECT_TRUE(c.called_0);
	EXPECT_TRUE(c.called_1);
	EXPECT_TRUE(c.called_2);
	EXPECT_TRUE(c.called_3);
}

TEST(Signal, callable_obj) {
	Signal<void(int, float)> sig;
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
	Signal<void(int, float)> sig;
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
	Signal<void(int, float)> sig;
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

TEST(Signal, derive) {
	Signal<void()> sig;
	
	struct Base0 {
		int data{ 0 };
		void f() {
			EXPECT_EQ(data, 0);
		}
	};
	struct Base1 {
		int data{ 1 };
		void f() {
			EXPECT_EQ(data, 1);
		}
	};
	struct Derived : Base0, Base1 {};
	Derived d;
	sig.Connect<&Base0::f>(&d);
	sig.Connect<&Base1::f>(&d);
	sig.Emit();
}

TEST(Signal, two_api) {
	Signal<void()> sig;

	struct A {
		int data = 0;
		void f(){
			data = 1;
		}
	};
	A a;
	sig.Connect<&A::f>(&a);
	sig.Disconnect(&A::f, &a);
	sig.Emit();
	EXPECT_EQ(a.data, 0);
}

TEST(Signal, const) {
	Signal<void()> sig;
	struct A {
		bool& called;
		void f() const {
			called = true;
		}
	};
	bool called = false;
	const A a{ called };
	sig.Connect<&A::f>(&a);
	sig.Emit();
	EXPECT_TRUE(called);
}

TEST(Signal, memslot) {
	Signal<void()> sig;
	struct A {
		bool called = false;
	};
	A a;
	sig.Connect([](A& a) {
		a.called = true;
	}, &a);
	sig.Emit();
	EXPECT_TRUE(a.called);
}

TEST(Signal, move) {
	Signal<void()> sig;
	struct A {
		bool called = false;
		void f() { called = true; }
	};
	int cnt = 0;
	struct CopyCnt {
		int& cnt;
		CopyCnt(int& cnt) :cnt{ cnt } {}
		CopyCnt(const CopyCnt& other) : cnt{ other.cnt } { cnt++; }
		CopyCnt(CopyCnt&& other) noexcept : cnt{ other.cnt } {}
		CopyCnt& operator=(const CopyCnt& other) {
			cnt = other.cnt;
			cnt++;
			return *this;
		}
		CopyCnt& operator=(CopyCnt&& other) noexcept {
			cnt = other.cnt;
			return *this;
		}
	};
	A a0, a1;
	sig.Connect([copycnt = CopyCnt{ cnt }](A& a){
		a.called = true;
	}, &a0);
	sig.MoveInstance(&a1, &a0);
	sig.Emit();
	EXPECT_FALSE(a0.called);
	EXPECT_TRUE(a1.called);
	EXPECT_EQ(cnt, 0);
}

TEST(Signal, acc) {
	Signal<int()> getints;
	getints.Connect([]() {return 1; });
	getints.Connect([]() {return 2; });
	getints.Connect([]() {return 3; });
	struct Acc {
		int sum = 0;
		void operator()(int v) {
			sum += v;
		}
	};
	Acc acc;
	getints.Emit(acc);
	EXPECT_EQ(acc.sum, 6);
}

TEST(Signal, scope) {
	Signal<void()> sig;
	int cnt = 0;
	{
		ScopedConnection conn{ sig.Connect([&]() {cnt++; }), &sig };
		sig.Emit();
		EXPECT_EQ(cnt, 1);
	}
	sig.Emit();
	EXPECT_EQ(cnt, 1);
}

TEST(Signal, scope_move) {
	struct A {
		A() = default;
		A(A&& a) noexcept : data{ a.data }, dataChanged { std::move(a.dataChanged) }, moved{ std::move(a.moved) }
		{ moved.Emit(this); }

		void SetData(int data) {
			if (this->data != data) {
				this->data = data;
				dataChanged.Emit(data);
			}
		}

		int data{ 0 };

		Signal<void(int)> dataChanged;
		Signal<void(A*)> moved;
	};
	struct DataTracker {
		DataTracker(A& a) :
			data_conn{ a.dataChanged.ScopeConnect([](DataTracker& t, int d) {
				t.data = d;
			}, this) },
			move_conn{ a.moved.ScopeConnect([](DataTracker& t, A* a) {
				t.data_conn.signal = &a->dataChanged;
				t.move_conn.signal = &a->moved;
			}, this) } {}

		DataTracker(DataTracker&& t) noexcept :
			data{ t.data }, data_conn{ std::move(t.data_conn) }, move_conn{ std::move(t.move_conn) }
		{
			data_conn.MoveInstance(this);
			move_conn.MoveInstance(this);
		}

		ScopedConnection<void(int)> data_conn;
		ScopedConnection<void(A*)> move_conn;
		int data{ 0 };
	};

	A a;
	{
		DataTracker t(a);
		A a2(std::move(a));
		DataTracker t2(std::move(t));
		a2.SetData(2);
		EXPECT_EQ(t.data, 0);
		EXPECT_EQ(t2.data, 2);
	}
}


template<typename T>
concept MemberSignalTest0 = requires(T t) {
	{ decltype(t.signal)(std::move(t.signal)) };
};
template<typename T>
concept MemberSignalTest1 = requires(T t) {
	{ t.signal.Emit() };
};
template<typename T>
concept MemberSignalTest2 = requires(T t) {
	{ t.signal.Clear() };
};
template<typename T>
concept MemberSignalTest3 = requires(T a, T b) {
	{ a.signal.Swap(b.signal) };
};

TEST(Signal, member) {
	struct A {
		MSignal<A, void()> signal;
		void EmitSignal() { signal.Emit(); }
	};
	struct B {
		Signal<void()> signal;
	};
	EXPECT_FALSE(MemberSignalTest0<A>);
	EXPECT_FALSE(MemberSignalTest1<A>);
	EXPECT_FALSE(MemberSignalTest2<A>);
	EXPECT_FALSE(MemberSignalTest3<A>);
	EXPECT_TRUE(MemberSignalTest0<B>);
	EXPECT_TRUE(MemberSignalTest1<B>);
	EXPECT_TRUE(MemberSignalTest2<B>);
	EXPECT_TRUE(MemberSignalTest3<B>);
	A a;
	bool called = false;
	a.signal.Connect([&called] {called = true; });
	a.EmitSignal();
	EXPECT_TRUE(called);
}

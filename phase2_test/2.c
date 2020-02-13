void test(int a, int b, int c, int d, int e, int f) {
	a = 1;
	b = 2;
	c = a + b;
	d = b + a;
	e = a * b;
	f = b * a;
	c = c * c;
	f = f * e;
}

int main() {
	test(1,2,3,4,5,6);
	return 0;
}


void test(int a, int b, int c, int d, int e, int f) {
	c = a + b;
	d = a * c;
	e = a;
	a = a + 1;
	d = a + b;
	f = e + b;
	f = e * f;
}                          

int main() {
	test(1,2,3,4,5,6);
	return 0;
}

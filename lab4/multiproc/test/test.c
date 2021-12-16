void foo();

int a = 1;

int main() {
	a = 2;
	a = 3;
	int b = a;
	foo();
	a = 4;
	return 0;
}

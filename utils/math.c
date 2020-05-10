int pow(int base, int power){
	int r = 1;
	for (int p_i = power; p_i; p_i--){
		r *= base;
	}
	return r;
}

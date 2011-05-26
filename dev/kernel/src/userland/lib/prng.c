unsigned int random( unsigned int* seed )
{
	*seed = 36969 * ((*seed) & 65535) + ((*seed) >> 16);

	return *seed;
}

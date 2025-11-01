#include <Networking/Networking.h>

int main()
{
	NS::Networking* Net = NS::Networking::Get();
	Net->Listen();
	return 0;
}  
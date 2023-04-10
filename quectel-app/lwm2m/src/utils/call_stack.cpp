#include <utils/CallStack.h>
using namespace android;
extern "C" void dump_stack(const char* logtag)
{
    CallStack stack(logtag);
}

#ifndef REBOX_SHARED_INSTANCE_FWD_HPP
#define REBOX_SHARED_INSTANCE_FWD_HPP

namespace rebox
{
    class throw_invalid_argument;

    template<typename T, typename Report = throw_invalid_argument>
    class shared_instance;
}

#endif

#include "zh/original_support/ww_support.h"
#include "../../GeneralsMD/Code/Libraries/Source/WWVegas/WWLib/BUFF.H"

#include <iostream>
#include <stdexcept>
#include <string>

int main()
{
    try {
        Buffer owned(32L);
        if (!owned.Is_Valid() || owned.Get_Size()!=32) throw std::runtime_error("owned Buffer allocation failed");
        auto* bytes=static_cast<char*>(owned.Get_Buffer());
        bytes[0]='W';
        owned.Reset();
        if (owned.Is_Valid() || owned.Get_Size()!=0) throw std::runtime_error("owned Buffer reset failed");
        char borrowed_bytes[4]={'A','B','C','D'};
        Buffer borrowed(borrowed_bytes,4);
        borrowed.Reset();
        if (borrowed_bytes[0]!='A' || borrowed_bytes[3]!='D')
            throw std::runtime_error("borrowed Buffer storage was changed");
        Buffer reassigned(8L);
        reassigned=Buffer(borrowed_bytes,4);
        if (reassigned.Get_Buffer()!=borrowed_bytes || reassigned.Get_Size()!=4)
            throw std::runtime_error("Buffer reassignment lost borrowed storage");
        const auto witness = zh::original_support::run_ww_support_probe();
        if (!zh::original_support::rejects_truncated_chunk()) throw std::runtime_error("truncated chunk negative control changed");
        std::cout << witness << '\n';
        return 0;
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}

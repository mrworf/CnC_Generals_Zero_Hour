#ifndef ZH_COMPONENT_NAME
#error "ZH_COMPONENT_NAME must name the bootstrap component"
#endif

#ifndef ZH_COMPONENT_SYMBOL
#error "ZH_COMPONENT_SYMBOL must provide a unique bootstrap symbol"
#endif

const char* ZH_COMPONENT_SYMBOL() noexcept
{
    return ZH_COMPONENT_NAME;
}

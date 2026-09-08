
#ifdef NDEBUG
#define GIVM_ASSERT(x)
#else
#define GIVM_ASSERT(x) (::givm::assert)(x, #x)
#endif

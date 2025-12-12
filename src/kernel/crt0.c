typedef void (*func_ptr)();

extern func_ptr __CTOR_LIST__;
extern func_ptr __CTOR_END__;
extern func_ptr __DTOR_LIST__;
extern func_ptr __DTOR_END__;

void _init(void) {
    for (func_ptr *p = &__CTOR_LIST__; p < &__CTOR_END__; p++) {
        if (*p != (func_ptr)0xFFFFFFFF && *p != (func_ptr)0)
            (*p)();
    }
}
void _fini(void) {
    for (func_ptr *p = &__DTOR_LIST__; p < &__DTOR_END__; p++) {
        if (*p != (func_ptr)0xFFFFFFFF && *p != (func_ptr)0)
            (*p)();
    }
}

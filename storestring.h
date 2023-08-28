// all storeString functions work with strings of any length.

// individual string functions:

// Free any previous string
// then allocate and copy new source.
// RETURN_OK if new string copied correctly
// RETURN_FAIL if no dest provided, or result is NULL
// NULL result may be from lack of source or alloc failure
int32 storeString(STRPTR *dest, STRPTR source);

// GetVar() into storeString()
// user must eventually FreeVec() dest.
// or storeString(&Dest, NULL) to free memory
// RETURN_OK or RETURN_FAIL
int32 storeVar(STRPTR *dest, CONST_STRPTR var_name, uint32 flags);

// storeString() into dest if (arg && *arg)
// else leave dest unchanged
// RETURN_FAIL if !arg or !*arg
// else return from storeString()
int32 storeArg(STRPTR *dest, int32 *arg);

// Add source to any existing string in dest
// RETURN_OK if new string copied correctly
// RETURN_WARN if no source
// RETURN_FAIL if no dest provided, or alloc failure
int32 storeStrCat(STRPTR *dest, STRPTR source);

// TRUE if we have a non-null value
int32 haveString(STRPTR *storage);

// Manage lists of strings

// add string to given list,
// RETURN_OK or RETURN_FAIL
int32 addStringList(STRPTR string, struct List *lst);

// stores result using storeString into STRPTR pointer
// RETURN_OK or RETURN_FAIL
int32 remStringList(STRPTR *storage, struct List *lst);

// Free all strings allocated with StoreString()
// storeVar() and string Lists
void freeStrings(void);



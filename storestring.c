// storeString.c
// dynamic string management

#include <string.h>
#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/utility.h>
#include "storestring.h"

static APTR stringPool = NULL;

struct StringNode
{
	struct Node node;	// ln_Name points to buffer
	char buffer[2];
};

// Free any previous string
// then allocate and copy new source.
// RETURN_OK if new string copied correctly
// RETURN_FAIL if no dest provided, or result is NULL
// NULL result may be from lack of source or alloc failure
int32 storeString(STRPTR *dest, STRPTR source);

// GetVar() into storeString()
// RETURN_OK or RETURN_FAIL
int32 storeVar(STRPTR *dest, CONST_STRPTR var_name, uint32 flags);

// storeString() into dest if (var_name && *var_name)
// else leave dest unchanged
// RETURN_FAIL if !arg or !*arg
// else return from storeString()
int32 storeArg(STRPTR *dest, int32 *arg);

// TRUE if we have a non-null value
int32 haveString(STRPTR *storage);

// add string to given list,
// RETURN_OK if all went well
// RETURN_WARN if NULL or empty string
// RETURN_FAIL for allocation failure
int32 addStringList(STRPTR string, struct List *lst);

// return next string value from list head
// RETURN_OK if string value assigned to storage
// RETURN_WARN for empty list
// RETURN_FAIL for allocation failure
int32 remStringList(STRPTR *storage, struct List *lst);

// Free all allocated strings
void freeStrings(void);

// Allocates a buffer, copies given string to it.
// returns NULL on any failure, NULL pointer or NULL string
// caller is responsible for IExec->FreeVec(result)
static STRPTR strAllocCopy(CONST_STRPTR string);


// Free any previous string
// then allocate and copy new source.
// RETURN_OK if new string copied correctly
// RETURN_WARN if no source (will still free old value)
// RETURN_FAIL if no dest provided, or alloc failure
// Call with NULL source to free previous value
int32 storeString(STRPTR *dest, STRPTR source)
{
	if(NULL == dest)
	{
		return(RETURN_FAIL);
	}

	if(*dest)
	{
		IExec->FreeVecPooled(stringPool, *dest);
		*dest = NULL;
		if((! source)||(! strlen(source)))
		{
			return(RETURN_WARN);
		}
	}

	if(NULL != (*dest = strAllocCopy(source)))
	{
		return(RETURN_OK);
	}

	return(RETURN_FAIL);
}

// GetVar() into storeString()
// if var not found, dest is not changed.
// RETURN_OK if varName found and stored
// RETURN_WARN if varName not found (dest will be unchanged)
// RETURN_FAIL for allocation failure
int32 storeVar(STRPTR *dest, CONST_STRPTR var_name, uint32 flags)
{
	STRPTR result;
	int32 result_size = 1024, return_value = RETURN_WARN;

	// allocate a buffer to receive GetVar() data
	result = IExec->AllocVecTags(result_size, TAG_END);
	if(NULL == result)
	{
		return(RETURN_FAIL);
	}

	if(0 < IDOS->GetVar(var_name, result, result_size, flags))
	{
		if(IDOS->IoErr() > (result_size - 1))
		{	// increase buffer size and try again
			IExec->FreeVec(result);
			result_size = IDOS->IoErr() + 1;
			result = IExec->AllocVecTags(result_size);
			if(NULL == result)
			{
				return(RETURN_FAIL);
			}
			
			if(1 > IDOS->GetVar(var_name, result, result_size, flags))
			{
				return(RETURN_FAIL);
			}
		}
		return_value = storeString(dest, result);
		IExec->FreeVec(result);
	}
	return(return_value);
}

// storeString() into dest if (arg && *arg)
// else leave dest unchanged
// RETURN_FAIL if !arg or !*arg
// else return from storeString()
int32 storeArg(STRPTR *dest, int32 *arg)
{
	if((arg)&&(*arg))
	{
		return(storeString(dest, (STRPTR)arg));
	}
	return(RETURN_FAIL);
}

// Add source to any existing string in dest
// RETURN_OK if new string copied correctly
// RETURN_WARN if no source
// RETURN_FAIL if no dest provided, or alloc failure
int32 storeStrCat(STRPTR *dest, STRPTR source)
{
	int32 totalLen = 0;
	char *localbuffer = NULL;

	if(NULL == source)
	{
		return(RETURN_WARN);
	}

	if(haveString(dest))
	{
		totalLen = strlen(*dest);
	}

	totalLen += strlen(source);

	if((localbuffer = IExec->AllocVecTags(totalLen + 1, TAG_END)))
	{
		int32 response;
		strcpy(localbuffer, *dest);
		strcat(localbuffer, source);
		response = storeString(dest, localbuffer);
		IExec->FreeVec(localbuffer);
		return(response);
	}
	return(RETURN_FAIL);
}

// TRUE if we have a non-null value
int32 haveString(STRPTR *storage)
{
	if(storage && *storage)
	{
		return(TRUE);
	}

	return(FALSE);
}

// add string to given list,
// RETURN_OK if all went well
// RETURN_WARN if NULL or empty string
// RETURN_FAIL for allocation failure, or NULL list
int32 addStringList(STRPTR string, struct List *lst)
{
	int16 len;
	struct StringNode *sn;

	if(NULL == lst)
	{
		return(RETURN_FAIL);
	}

	if((NULL == string)||(0 == (len = strlen(string))))
	{
		return(RETURN_WARN);
	}

	if(NULL == stringPool)
	{
		if(NULL == (stringPool = IExec->AllocSysObject(
			ASOT_MEMPOOL, TAG_END)))
		{
			return(RETURN_FAIL);
		}
	}

	sn = IExec->AllocVecPooled(stringPool, len + sizeof(struct StringNode));
	if(NULL == sn)
	{
		return(RETURN_FAIL);
	}

	sn->node.ln_Name = (STRPTR)&sn->buffer[0];
	IUtility->Strlcpy((STRPTR)sn->buffer, string, len + 1);

	IExec->AddTail(lst, &sn->node);

	return(RETURN_OK);
}

// return next string value from list head
// RETURN_OK if string value assigned to storage
// RETURN_WARN for empty list
// RETURN_FAIL for allocation failure or no list
int32 remStringList(STRPTR *storage, struct List *lst)
{
	struct StringNode *sn;
	int32 result = RETURN_WARN;

	if(NULL == lst)
	{
		return(RETURN_FAIL);
	}

	sn = (struct StringNode *)IExec->RemHead(lst);

	if(sn)
	{
		result = storeString(storage, sn->node.ln_Name);
		IExec->FreeVecPooled(stringPool, sn);
	}
	else if(*storage)
	{
		storeString(storage, NULL);
	}

	return(result);
}

// Free all stored Strings, stored Vars,
// including StringList contents
void freeStrings(void)
{
	if(stringPool)
	{
		IExec->FreeSysObject(ASOT_MEMPOOL, stringPool);
		stringPool = NULL;
	}
}


// Allocates a buffer, copies given string to it.
// returns NULL on any failure, NULL pointer or NULL string
// caller is responsible for IExec->FreeVec(result)
static STRPTR strAllocCopy(CONST_STRPTR string)
{
	uint32 length;

	if((NULL == string) || (0 == ((length = strlen(string)))))
	{
		return(NULL);
	}

	if(NULL == stringPool)
	{
		if(NULL == (stringPool = IExec->AllocSysObject(
			ASOT_MEMPOOL, TAG_END)))
		{
			return(NULL);
		}
	}

	length++;	// space for NULL termination

	STRPTR new_string = IExec->AllocVecPooled(stringPool, length);

	if(new_string)
	{
		IUtility->Strlcpy(new_string, (STRPTR)string, length);
	}

	return(new_string);
}




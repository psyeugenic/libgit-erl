/*
 * Copyright (c) 2010 Björn-Egil Dahlberg
 *
 * Created: 2010-12-15
 * Author:  Björn-Egil Dahlberg
 *
 */

#include "erl_nif.h"
#include <stdio.h>
#include <string.h>
#include <git2.h>
#include "libgit_util.h"

ERL_NIF_TERM egit_oid_to_binary(ErlNifEnv *env, const git_oid *id) {
    ErlNifBinary ibin;

    enif_alloc_binary(40, &ibin);
    git_oid_fmt((char *)ibin.data, id);

    return enif_make_binary(env, &ibin);
}

int egit_oid_from_binary(ErlNifEnv *env, ERL_NIF_TERM sha, git_oid *id) {
    ErlNifBinary ibin;
    char hex[41];

    if(!enif_inspect_binary(env, sha, &ibin))
	return 0;

    if(ibin.size > 40)
	return 0;

    memcpy(hex, ibin.data, ibin.size);
    hex[ibin.size + 1] = '\0';

    git_oid_mkstr(id, hex);

    return 1;
}



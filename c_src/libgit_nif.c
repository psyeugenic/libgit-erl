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
/*
#include <git/commit.h>
#include <git/tag.h>
#include <git/common.h>
#include <git/errors.h>
#include <git/index.h>
#include <git/odb.h>
#include <git/oid.h>
#include <git/revwalk.h>
#include <git/repository.h>
#include <git/zlib.h>
*/

#define MAXPATH (1024)

static ErlNifResourceType *rtype_git_repository;
static ErlNifResourceType *rtype_git_revwalk;
static ErlNifResourceType *rtype_git_object;
static ErlNifResourceType *rtype_git_commit;
static ErlNifResourceType *rtype_git_tree;

/* useful atoms */
static ERL_NIF_TERM am_git_repository;
static ERL_NIF_TERM am_git_revwalk;
static ERL_NIF_TERM am_git_commit;
static ERL_NIF_TERM am_git_object;
static ERL_NIF_TERM am_git_tree;
static ERL_NIF_TERM am_git_otype;
static ERL_NIF_TERM am_error;
static ERL_NIF_TERM am_ok;
/* commit atoms */
static ERL_NIF_TERM am_committer;
static ERL_NIF_TERM am_subject;
static ERL_NIF_TERM am_message;
static ERL_NIF_TERM am_parents;
static ERL_NIF_TERM am_author;
static ERL_NIF_TERM am_email;
static ERL_NIF_TERM am_name;
static ERL_NIF_TERM am_time;
static ERL_NIF_TERM am_tree; /* not the reference */

static void init(ErlNifEnv *env) {
    /* useful atoms */
    am_error      = enif_make_atom(env, "error");
    am_ok         = enif_make_atom(env, "ok");

    /* reference atoms */
    am_git_repository = enif_make_atom(env, "git_repository");
    am_git_revwalk    = enif_make_atom(env, "git_revwalk");
    am_git_object     = enif_make_atom(env, "git_object");
    am_git_commit     = enif_make_atom(env, "git_commit");
    am_git_tree       = enif_make_atom(env, "git_tree");
    am_git_otype      = enif_make_atom(env, "git_otype");

    /* commit atoms */
    am_committer  = enif_make_atom(env, "committer");
    am_subject    = enif_make_atom(env, "subject");
    am_parents    = enif_make_atom(env, "parents");
    am_message    = enif_make_atom(env, "message");
    am_author     = enif_make_atom(env, "author");
    am_email      = enif_make_atom(env, "email");
    am_time       = enif_make_atom(env, "time");
    am_name       = enif_make_atom(env, "name");
    am_tree       = enif_make_atom(env, "tree"); /* not the reference */
}

/* pointer wrappers */
typedef struct {
    git_repository *repo;
} egit_repository_t;

typedef struct {
    git_revwalk *walker;
} egit_revwalk_t;

typedef struct {
    git_object *object;
} egit_object_t;

typedef struct {
    git_commit *commit;
} egit_commit_t;

typedef struct {
    git_tree *tree;
} egit_tree_t;

/* garbage collect callbacks */
static void gc_git_repository(ErlNifEnv* env, void* data) {
    //fprintf(stderr, "gc_git_repository\r\n");
    /* flag in egit_* for gc:able (freeable) ? */
    git_repository_free( ((egit_repository_t *)data)->repo);
}

static void gc_git_revwalk(ErlNifEnv* env, void* data) {
    //fprintf(stderr, "gc_git_revwalk\r\n");
    git_revwalk_free( ((egit_revwalk_t *)data)->walker);
}

static void gc_git_object(ErlNifEnv* env, void* data) {
    //fprintf(stderr, "gc_git_object\r\n");
}

static void gc_git_commit(ErlNifEnv* env, void* data) {
    //fprintf(stderr, "gc_git_commit\r\n");
}

static void gc_git_tree(ErlNifEnv* env, void* data) {
    //fprintf(stderr, "gc_git_tree\r\n");
}


/* get and check tuples */

static int get_resource_from_tuple(ErlNifEnv *env, ERL_NIF_TERM tuple, ERL_NIF_TERM type, ErlNifResourceType *rtype, void **data)
{
    int arity;
    const ERL_NIF_TERM *arr;

    // fprintf(stderr, "get_tuple\r\n");

    if (!enif_get_tuple(env, tuple, &arity, &arr))
	return 0;

    // fprintf(stderr, "check atom and get resource\r\n");
    if (!enif_is_identical(arr[0], type) ||
	    !enif_get_resource(env, arr[1], rtype, data))
	return 0;

    return 1;
}

/* 
 * REPOSITORY.H
 */

static ERL_NIF_TERM repository_open(ErlNifEnv *env, int argc, const ERL_NIF_TERM argv[])
{
    char path[MAXPATH];
    git_repository *repo = NULL;
    egit_repository_t *data;
    ErlNifBinary ibin;
    ERL_NIF_TERM result;
    int res = 0;

    if(!enif_inspect_binary(env, argv[0], &ibin))
	return enif_make_badarg(env);

    if (ibin.size > MAXPATH - 1)
	return enif_make_badarg(env);

    memcpy(path, ibin.data, ibin.size);
    path[ibin.size] = '\0';

    if (( res = git_repository_open(&repo, path)) < 0)
	return enif_make_tuple2(env, am_error, enif_make_string(env, git_strerror(res), ERL_NIF_LATIN1));

    data = (egit_repository_t *) enif_alloc_resource(rtype_git_repository, sizeof(egit_repository_t));
    data->repo = repo;

    result = enif_make_resource(env, data);
    enif_release_resource(data);

    return enif_make_tuple2(env, am_ok, enif_make_tuple2(env, am_git_repository, result));
}


/* repository_lookup(repo(), sha()) -> {ok, object()} | {error, Error}. */
static ERL_NIF_TERM repository_lookup(ErlNifEnv *env, int argc, const ERL_NIF_TERM argv[])
{
    egit_repository_t *repo;
    egit_object_t *data;
    git_oid id;
    git_object *object = NULL;
    ERL_NIF_TERM result;

    if (!get_resource_from_tuple(env, argv[0], am_git_repository, rtype_git_repository, (void **)&repo))
	return enif_make_badarg(env);
 
    if (!egit_oid_from_binary(env, argv[1], &id))
	return enif_make_badarg(env);

    git_repository_lookup(&object, repo->repo, &id, GIT_OBJ_ANY);

    data = (egit_object_t *) enif_alloc_resource(rtype_git_object, sizeof(egit_object_t));
    data->object = object;

    result = enif_make_resource(env, data);
    enif_release_resource(data);
    return enif_make_tuple2(env, am_git_object, result);
}

/*
 * REVWALK.h
 */
static ERL_NIF_TERM revwalk_new(ErlNifEnv *env, int argc, const ERL_NIF_TERM argv[])
{
    git_revwalk *walker = NULL;
    egit_revwalk_t *data;
    ERL_NIF_TERM result;
    egit_repository_t *repo;
    int res;

    if (argc != 1 || !get_resource_from_tuple(env, argv[0], am_git_repository, rtype_git_repository, (void **)&repo))
	return enif_make_badarg(env);
 
    if (( res = git_revwalk_new(&walker, repo->repo)) < 0)
	return enif_make_tuple2(env, am_error, enif_make_string(env, git_strerror(res), ERL_NIF_LATIN1));

    data = (egit_revwalk_t *) enif_alloc_resource(rtype_git_revwalk, sizeof(egit_revwalk_t));
    data->walker = walker;

    result = enif_make_resource(env, data);
    enif_release_resource(data);

    return enif_make_tuple2(env, am_ok, enif_make_tuple2(env, am_git_revwalk, result));
}

static ERL_NIF_TERM revwalk(ErlNifEnv *env, int argc, const ERL_NIF_TERM argv[])
{
    ERL_NIF_TERM result;
    egit_commit_t *ecommit;
    egit_commit_t *wcommit;
    egit_repository_t *erepo;
    git_revwalk *walker = NULL;
    git_commit *commit = NULL;

    if (!get_resource_from_tuple(env, argv[0], am_git_repository, rtype_git_repository, (void **)&erepo) || !get_resource_from_tuple(env, argv[1], am_git_commit, rtype_git_commit, (void **)&ecommit))
	return enif_make_badarg(env);
 
    git_revwalk_new(&walker, erepo->repo);
    git_revwalk_sorting(walker, GIT_SORT_TIME);
    
    git_revwalk_push(walker, ecommit->commit);

    result = enif_make_list(env, 0);

    while ((commit = git_revwalk_next(walker)) != NULL) {
	wcommit = (egit_commit_t *) enif_alloc_resource(rtype_git_commit, sizeof(egit_commit_t));
	wcommit->commit = commit;
	result  = enif_make_list_cell(env, 
		enif_make_tuple2(env, am_git_commit, enif_make_resource(env, wcommit)), result);
	enif_release_resource(wcommit);
    }

    git_revwalk_free(walker);
    return result;
}



/*
 * TREE.h
 */

/* tree_id(git_tree()) -> sha().  */
static ERL_NIF_TERM tree_id(ErlNifEnv *env, int argc, const ERL_NIF_TERM argv[])
{
    egit_tree_t *etree;
    git_oid *id;

    if (!get_resource_from_tuple(env, argv[0], am_git_tree, rtype_git_tree, (void **)&etree))
	return enif_make_badarg(env);
 
    id = (git_oid *)git_tree_id(etree->tree);

    return egit_oid_to_binary(env, id);
}

/* tree_lookup(git_repository(), sha()) -> git_tree(). */
static ERL_NIF_TERM tree_lookup(ErlNifEnv *env, int argc, const ERL_NIF_TERM argv[])
{
    egit_repository_t *erepo;
    egit_tree_t *etree;
    git_oid id;
    git_tree *tree = NULL;

    ERL_NIF_TERM result;

    if (!get_resource_from_tuple(env, argv[0], am_git_repository, rtype_git_repository, (void **)&erepo))
	return enif_make_badarg(env);
 
    if (!egit_oid_from_binary(env, argv[1], &id))
	return enif_make_badarg(env);

    if (git_tree_lookup(&tree, erepo->repo, &id) < 0)
	return enif_make_badarg(env);

    etree = (egit_tree_t *) enif_alloc_resource(rtype_git_tree, sizeof(egit_tree_t));
    etree->tree = tree;

    result = enif_make_resource(env, etree);
    enif_release_resource(etree);
    return enif_make_tuple2(env, am_git_tree, result);
}



/* tree_entrycount({git_tree, <<tree>>}) ->
 *     integer().
 */
static ERL_NIF_TERM tree_entrycount(ErlNifEnv *env, int argc, const ERL_NIF_TERM argv[])
{
    egit_tree_t *tree;
    size_t count;

    if (argc != 1 || !get_resource_from_tuple(env, argv[0], am_git_tree, rtype_git_tree, (void **)&tree))
	return enif_make_badarg(env);
 
    count = git_tree_entrycount(tree->tree);

    return enif_make_ulong(env, count);
}

/*
 * COMMIT.H
 */

static ERL_NIF_TERM string_to_binary(ErlNifEnv *env, const char *string)
{
    ErlNifBinary ibin;
    int len;

    len = strlen(string);
    enif_alloc_binary(len, &ibin);
    memcpy(ibin.data, string, len);
 
    return enif_make_binary(env, &ibin);
}
/* commit_id({git_commit, <<tree>>}) -> sha().
 */
static ERL_NIF_TERM commit_id(ErlNifEnv *env, int argc, const ERL_NIF_TERM argv[])
{
    egit_commit_t *ecommit;
    git_oid *oid;

    if (!get_resource_from_tuple(env, argv[0], am_git_commit, rtype_git_commit, (void **)&ecommit))
	return enif_make_badarg(env);
 
    oid = (git_oid *)git_commit_id(ecommit->commit);

    return egit_oid_to_binary(env, oid);
}


static ERL_NIF_TERM commit_lookup_build_person(ErlNifEnv *env, const git_person *person)
{
    ERL_NIF_TERM result;

    result = enif_make_list(env, 0);
    result = enif_make_list_cell(env, enif_make_tuple2(env, am_name, 
		string_to_binary(env, git_person_name((git_person *)person))), result);
    result = enif_make_list_cell(env, enif_make_tuple2(env, am_email,
		string_to_binary(env, git_person_email((git_person *)person))), result);
    result = enif_make_list_cell(env, enif_make_tuple2(env, am_time,
		enif_make_ulong(env, (unsigned long) git_person_time((git_person *)person))), result);

    return result;
}

/*
 * commit_to_term({git_commit, <<commit>>}) ->
 *     term()
 */
static ERL_NIF_TERM commit_to_term(ErlNifEnv *env, int argc, const ERL_NIF_TERM argv[])
{
    egit_commit_t *ecommit, *ecommit_parent;
    egit_tree_t *etree;
    git_commit *commit;
    unsigned int np = 0, i;
    ERL_NIF_TERM result;
    ERL_NIF_TERM parents;

    result = enif_make_list(env, 0);
    
    if (argc != 1 || !get_resource_from_tuple(env, argv[0], am_git_commit, rtype_git_commit, (void **)&ecommit))
	return enif_make_badarg(env);
    commit = ecommit->commit;

    /* message */
    result = enif_make_list_cell(env, enif_make_tuple2(env, am_message, 
		string_to_binary(env, git_commit_message(commit))), result);

    /* subject */
    result = enif_make_list_cell(env, enif_make_tuple2(env, am_subject, 
		string_to_binary(env, git_commit_message_short(commit))), result);

    /* parents */
    parents = enif_make_list(env, 0);
    np = git_commit_parentcount(commit);
    for (i = 0; i < np; ++i) {
	ecommit_parent = (egit_commit_t *) enif_alloc_resource(rtype_git_commit, sizeof(egit_commit_t));
	ecommit_parent->commit = (git_commit *)git_commit_parent(commit, i);
 
	parents = enif_make_list_cell(env, enif_make_tuple2(env, am_git_commit, 
		    enif_make_resource(env, ecommit_parent)), parents);

	enif_release_resource(ecommit_parent);
    }

    result = enif_make_list_cell(env, enif_make_tuple2(env, am_parents, 
		parents), result);

    /* committer */
    result = enif_make_list_cell(env, enif_make_tuple2(env, am_committer, 
		commit_lookup_build_person(env, git_commit_committer(commit))), result);

    /* author */
    result = enif_make_list_cell(env, enif_make_tuple2(env, am_author, 
		commit_lookup_build_person(env, git_commit_author(commit))), result);

    /* tree */
    etree = (egit_tree_t *) enif_alloc_resource(rtype_git_tree, sizeof(egit_tree_t));
    etree->tree = (git_tree *)git_commit_tree(commit);
 
    result = enif_make_list_cell(env, enif_make_tuple2(env, am_tree, 
		enif_make_tuple2(env, am_git_tree, enif_make_resource(env, etree))), result);

    enif_release_resource(etree);
    return result;
}


/* commit_lookup(git_repository(), sha() :: binary()) -> git_commit() */
static ERL_NIF_TERM commit_lookup(ErlNifEnv *env, int argc, const ERL_NIF_TERM argv[])
{
    egit_repository_t *erepo;
    git_oid id;
    git_commit *commit = NULL;
    egit_commit_t *ecommit;
    ERL_NIF_TERM result;

    if (!get_resource_from_tuple(env, argv[0], am_git_repository, rtype_git_repository, (void **)&erepo))
	return enif_make_badarg(env);
 
    if (!egit_oid_from_binary(env, argv[1], &id))
	return enif_make_badarg(env);

    git_commit_lookup(&commit, erepo->repo, &id);
   
    ecommit = (egit_commit_t *) enif_alloc_resource(rtype_git_commit, sizeof(egit_commit_t));
    ecommit->commit = commit;
		
    result = enif_make_tuple2(env, am_git_commit, enif_make_resource(env, ecommit));

    enif_release_resource(ecommit);
    return result;
}

static ErlNifFunc nif_functions[] =
{
    /* commit.h */
    {"commit_id", 1, commit_id},
    {"commit_lookup", 2, commit_lookup},
    {"commit_to_term", 1, commit_to_term},

    /* tree.h */
    {"tree_id", 1, tree_id},
    {"tree_lookup", 2, tree_lookup},
    {"tree_entrycount", 1, tree_entrycount},

    /* revwalk.h */
    {"revwalk_new", 1, revwalk_new},
    {"revwalk", 2, revwalk},

    /* repository.h */
    {"repository_lookup", 2, repository_lookup},
    {"repository_open", 1, repository_open},
};

static int load(ErlNifEnv* env, void** priv_data, ERL_NIF_TERM load_info)
{
    init(env);
    rtype_git_repository = enif_open_resource_type(env,NULL,
	    "gc_git_repository",gc_git_repository,
	    ERL_NIF_RT_CREATE,NULL);

    rtype_git_revwalk = enif_open_resource_type(env,NULL,
	    "gc_git_revwalk",gc_git_revwalk,
	    ERL_NIF_RT_CREATE,NULL);

    rtype_git_object = enif_open_resource_type(env,NULL,
	    "gc_git_object", gc_git_object,
	    ERL_NIF_RT_CREATE,NULL);

    rtype_git_commit = enif_open_resource_type(env,NULL,
	    "gc_git_commit", gc_git_commit,
	    ERL_NIF_RT_CREATE,NULL);

    rtype_git_tree = enif_open_resource_type(env,NULL,
	    "gc_git_tree",gc_git_tree,
	    ERL_NIF_RT_CREATE,NULL);

    *priv_data = NULL;
    return 0;
}

ERL_NIF_INIT(libgit, nif_functions, load, NULL, NULL, NULL)

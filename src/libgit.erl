%
% Copyright (c) 2010 Björn-Egil Dahlberg
%
% Created: 2010-12-15
% Author:  Björn-Egil Dahlberg
%

-module(libgit).
-export([
	% repository.h
	repository/1,        % repository_open/1,
	repository_lookup/2,

	% commit.h
	commit_id/1,
	commit_lookup/2,

	commit_to_term/1,
	
	% tree.h
	tree_id/1,
	tree_lookup/2,
	tree_entrycount/1,
	
	% revwalk.h
	revwalk_new/1,
	revwalk/2
	
	% tag.h
	%tag_id/1,
    ]).

-on_load(load_git/0).

% repo()    :: {git_repository, <<repo>>    :: magic_binary()}
% revwalk() :: {git_revwalk,    <<revwalk>> :: magic_binary()}
% object()  :: {git_object,     <<object>>  :: magic_binary()}
% commit()  :: {git_commit,     <<commit>>  :: magic_binary()}
% tree()    :: {git_tree,       <<tree>>    :: magic_binary()}

-type git_repository() :: {git_repository, binary()}.
-type git_revwalk()    :: {git_revwalk, binary()}.
-type git_commit()     :: {git_commit, binary()}.
-type git_object()     :: {git_object, binary()}.
-type git_tree()       :: {git_tree, binary()}.
-type sha()            :: binary().

%% commit.h

-spec commit_id(git_commit()) -> sha().

commit_id(_Commit) -> 
    libgit_error().    


-spec commit_lookup(git_repository(), sha()) -> git_commit().

% known issue: i don't bump the refc of the repo object so it
% might get gc'ed. It's an easy fix though ... just lazy.
% results in core-dump in commit_lookup ...

commit_lookup(_Repository, _Sha) -> 
    libgit_error().    

-spec commit_to_term(git_commit()) -> term().

commit_to_term(_Commit) -> 
    libgit_error().    

%% repository.h

repository(Path) ->
    {ok, Repo} = repository_open(Path),
    Repo.

-spec repository_open(Path :: binary()) -> {ok, git_repository()} | {error, string()}.

repository_open(_Path) -> 
    libgit_error().    

-spec repository_lookup(git_repository(), sha()) -> git_object().

repository_lookup(_Repository, _Id) -> 
    libgit_error().    

%% tree.h

-spec tree_id(git_tree()) -> sha().

tree_id(_Tree) -> 
    libgit_error().    

-spec tree_lookup(git_repository(), sha()) -> git_tree().

tree_lookup(_Repository, _Sha) -> 
    libgit_error().    


-spec tree_entrycount(git_tree()) -> integer().

tree_entrycount(_Tree) -> 
    libgit_error().

%% revwalk.h

-spec revwalk_new(git_repository()) -> {ok, git_revwalk()}.

% will not need this
revwalk_new(_Val) -> 
    libgit_error().    

% should have (repo(), commit(), Sort, [hidden commits()])
-spec revwalk(git_repository(), git_commit()) -> [git_commit()].

revwalk(_Repository, _Commit) -> 
    libgit_error().    

% NIF handler

load_git() ->
    case code:priv_dir(libgit) of
        {error, bad_name} ->
            SoName = filename:join("priv", libgit_erl);
        Dir ->
            SoName = filename:join(Dir, libgit_erl)
    end,
    erlang:load_nif(SoName, 0).

libgit_error() -> undefined.


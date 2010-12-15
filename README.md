libgit2 erlang bindings
=======================

The title is an overstatement. It has some bindings to libgit2.


Example of use:
---------------
    > R = libgit:repository(<<"otp/.git">>).
    > C = libgit:commit_lookup(R, <<"0701070394cab463bf5ebbc69424e20778633a57">>).
    > io:format("~p~n", [libgit:commit_to_term(C)]).
	[{tree,{git_tree,<<>>}},
	 {author,[{time,1291201871},
		  {email,<<"psyeugenic@gmail.com">>},
		  {name,<<"Björn-Egil Dahlberg">>}]},
	 {committer,[{time,1291201871},
		     {email,<<"psyeugenic@gmail.com">>},
		     {name,<<"Björn-Egil Dahlberg">>}]},
	 {parents,[{git_commit,<<>>},{git_commit,<<>>}]},
	 {subject,<<"Merge branch 'egil/tools/eprof-badarith-timer-resolution/OTP-8963' into dev">>},
	 {message,<<"Merge branch 'egil/tools/eprof-badarith-timer-resolution/OTP-8963' into dev\n\n* egil/tools/eprof-badarith-timer-resolution/OTP-8963:\n  eprof: fix badarith exception on divide\n">>}]

    > [libgit:commit_id(Commit) || Commit <- libgit:revwalk(R, C)].
	[<<"84adefa331c4159d432d22840663c38f155cd4c1">>,
	 <<"0ad86bc3d062508f78142f2cc1a4756fece24c10">>,
	 <<"5c2bac05f515211a6158bb06d91ea3b0b3861b84">>,
	 <<"7bd64232f9c919a233b35cdaff850098a0e64d82">>,
	 ...]


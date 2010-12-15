{application, libgit, [
	{description, "libgit2 erlang bindings"},
	{vsn, "0.0.5"},
	{modules, [
		libgit
	    ]
	},
	{registered, []},
        {applications, [
	        kernel,
		stdlib
	    ]
	},
       {env, []}
    ]
}.

should be placed in a separated exe project, which links engine dll/lib.
placed in the same project for now to simplify our job.
anyway don't forget to use #ifdef EDITOR whenever you declare/define editor-specific logic.

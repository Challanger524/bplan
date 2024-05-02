message("Files for removal:")
string (REPLACE " " "\n  " FILES_LIST_ "${FILES_}")
message("  " ${FILES_LIST_})

file(REMOVE_RECURSE "${FILES_}")

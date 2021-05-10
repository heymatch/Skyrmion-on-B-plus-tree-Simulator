setting = 1

read_function_list = ["SEQUENTIAL", "RANGE_READ"]
search_function_list = ["TRAD_BINARY_SEARCH", "BIT_BINARY_SEARCH"]
update_function_list = ["OVERWRITE", "PERMUTATION_WRITE", "PERMUTATION_WRITE_MIGRATE"]
split_merge_function_list = ["TRAD", "UNIT"]

for split_merge_i in split_merge_function_list:
    for update_i in update_function_list:
        for search_i in search_function_list:
            for read_i in read_function_list:
                f = open("setting" + str(setting) + ".txt", "w")
                if split_merge_i == "TRAD":
                    f.write(
                        f"""word_length=64
track_length=2048
unit_size=1
kp_length=2
ordering=SORTED
read_function={read_i}
search_function={search_i}
update_function={update_i}
insert_function=SEQUENTIAL
delete_function=SEQUENTIAL
split_merge_function={split_merge_i}
                        """
                    )
                elif split_merge_i == "UNIT":
                    f.write(
                        f"""word_length=64
track_length=2048
unit_size=2
kp_length=3
ordering=SORTED
read_function={read_i}
search_function={search_i}
update_function={update_i}
insert_function=SEQUENTIAL
delete_function=SEQUENTIAL
split_merge_function={split_merge_i}
                        """
                    )

                setting = setting + 1
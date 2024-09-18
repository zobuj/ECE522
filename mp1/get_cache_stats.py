import os
 
def get_cache_info(cpu_num):
    cache_dir = f"/sys/devices/system/cpu/cpu{cpu_num}/cache"
    cache_info = {}

    for index in range(4):  # Adjust range based on the number of cache levels
        cache_level_dir = os.path.join(cache_dir, f"index{index}")

        if not os.path.exists(cache_level_dir):
            continue
 
        cache_info[index] = {}
        with open(os.path.join(cache_level_dir, "level"), "r") as f:
            cache_info[index]["level"] = f.read().strip()

        with open(os.path.join(cache_level_dir, "type"), "r") as f:
            cache_info[index]["type"] = f.read().strip()

        with open(os.path.join(cache_level_dir, "size"), "r") as f:
            cache_info[index]["size"] = f.read().strip()

        with open(os.path.join(cache_level_dir, "ways_of_associativity"), "r") as f:
            cache_info[index]["associativity"] = f.read().strip()

        with open(os.path.join(cache_level_dir, "shared_cpu_list"), "r") as f:
            cache_info[index]["shared_cpu_list"] = f.read().strip()

        with open(os.path.join(cache_level_dir, "coherency_line_size"), "r") as f:
            cache_info[index]["coherency_line_size"] = f.read().strip()

        with open(os.path.join(cache_level_dir, "number_of_sets"), "r") as f:
            cache_info[index]["number_of_sets"] = f.read().strip()

    return cache_info
 
for cpu in range(0,12): # Adjust range based on the number of CPUs
    print(f"CPU {cpu}:")
    cache_details = get_cache_info(cpu)

    for cache in cache_details:
        print(cache_details[cache])
    print()
                            
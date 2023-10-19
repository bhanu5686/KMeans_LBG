
#include "stdafx.h"

#define ld double 
#define delta 0.00001 
#define filename "Universe.csv"
#define CBSize 8
#define VecSize 12
ld tokhuraWeight[12] = {1.0, 3.0, 7.0, 13.0, 19.0, 22.0, 25.0, 33.0, 42.0, 50.0, 56.0, 61.0};
ld oldDist = 0.0;

//struct vector which will represent each vector 
typedef struct vector
{
	int cluster;
    double data[VecSize];
}vec;

//prints the vector which size is in variable row
void printVector(vec *x,int row)
{
	int i=0,j=0;
	for(i=0;i<row;i++)
	{
		for(j=0;j<VecSize;j++)
			printf("%lf ",x[i].data[j]);
		printf("\n");
	}
}

//used to read universe file into universe vector
bool ReadFile(vec *universe,const char  *fileName)
{
	
    	FILE *fp;
    	fopen_s(&fp, fileName, "r");
    	if(!fp)
      	return false;
    	else
    	{
    		char buffer[200];
    		ld val = 0.0;
    		int i=0,j=0;
			while(fgets(buffer, 200, fp))
        	{
        		j=0;
        		char *nextToken = NULL;
        		char* token = strtok_s(buffer, ",", &nextToken);
    			while (token != NULL) 
				{
    				val = strtod(token, NULL);
    				universe[i].data[j++] = val; 
        			token = strtok_s(NULL, ",", &nextToken);
    			}
    			universe[i++].cluster = -1;
        	}         
    	}
    	fclose(fp);
    	return true;
}

//used to write final codebook vector into output file
bool WriteFile(vec *x,const char  *fileName,int row)
{
	
    	FILE *fp;
    	fopen_s(&fp, fileName, "w");
    	if(!fp)
    	{
        	return false;
    	}
    	else
    	{
	  	int i=0,j=0;
	  	fprintf(fp,"index : data\n");
		for(i=0;i<row;i++)
		{
			fprintf(fp,"%d : ",i);
			for(j=0;j<VecSize;j++)
				fprintf(fp,"%lf ",x[i].data[j]);
			fprintf(fp,"\n");
		}             
    	}
    	fclose(fp);
    	return true;
}


//initialize codebook with random vectors from the universe
void initialize(vec *universe,vec *codebook,int row)
{
	int i=0;
	int x = row/(CBSize + 1);
	int y = x;
	for(i=0;i<CBSize;i++)
	{
		codebook[i]=universe[x];
		x += y;
	}
}

//return average tokhura distance calculated using formula studied in the class
ld tokhura(vec *x,vec *y)
{
	ld dist = 0.0;
	for(int i=0;i<VecSize;i++){
		dist += (tokhuraWeight[i] * ((*x).data[i]-(*y).data[i])*((*x).data[i]-(*y).data[i])); 
	}
	ld result = dist/VecSize;
	return result;
}

// calculate average distortion and returns its value
ld avgDistortion(vec *universe,vec *codebook,int row)
{
	ld tDist = 0.0;
	ld total = 0.0;
	int i=0,j=0,cnt=0;
	for(i=0;i<CBSize;i++)
	{
		cnt=0;
		for(j=0;j<row;j++)
		{
			if(universe[j].cluster == i)
			{
				//using formula studied in the class
				tDist = tDist + tokhura(&universe[j],&codebook[i]); 
				cnt++;
			}
		}
		total = total + (ld)cnt;
	}	
	ld avgDist = tDist/total;    
	return avgDist;
}


//classify universe vectors into cluster based on NN rule
void classifyRegion(vec *universe,vec *codebook,int row)
{
	int i=0,j=0,index=0;
	ld minDist = DBL_MAX, tDist = 0.0;
	for(i=0;i<row;i++)
	{
		minDist = DBL_MAX;
		for(j=0;j<CBSize;j++)
		{
			tDist = tokhura(&universe[i],&codebook[j]);			
			if(tDist < minDist)
			{       
				minDist = tDist;
				index = j;
			}
		}
		//updating cluster value with index
		universe[i].cluster = index;
	}
	return;
}

//update codebook vectors with the centroid of the universe vectors of the same cluster
void UpdateCodebook(vec *universe,vec *codebook,int row)
{
	int i=0,j=0,k=0;
	for(i=0;i<CBSize;i++)
	{
		ld temp[VecSize] = {0.0};
		int cnt=0;		
		for(j=0;j<row;j++)
		{	
			if(universe[j].cluster == i)
			{					
				for(k=0;k<VecSize;k++)
				{
					//store same index values in the temp vector
					temp[k] += universe[j].data[k];
				}
				cnt++;
			}	
		}
		//calculates centroid
		for(j=0;j<VecSize;j++)
		{
			codebook[i].data[j] = temp[j]/((ld)cnt);            
		}
	}
	return;
}

//kmeans algorith
void K_Means(vec *universe,vec *codebook,int row)
{
	//initialize no of iterations
	int m=0;

	//initialize codebook vectors using some adeqaute method
	initialize(universe,codebook,row);	

	//classifying universe vectors into same cluster
	classifyRegion(universe,codebook,row);

	//calculating average distortion
	ld newDist = avgDistortion(universe,codebook,row);			
		
	printf("Iteration is : %d\taverage Distortion is : %lf\t net Change : %lf\n",m,newDist,abs(newDist - oldDist));
	
	while(abs(newDist - oldDist) > delta)
	{
		//update codebook vector using centroid of the universe vector since clustering is already performed before this execution
		UpdateCodebook(universe,codebook,row);
		//clustering the universe vectors using NN rule i.e tokhura distance
		classifyRegion(universe,codebook,row);	
		oldDist = newDist;
		//calculate average distortion
		newDist = avgDistortion(universe,codebook,row);
		m++;
		printf("Iteration is : %d\taverage Distortion is : %lf\t net Change : %lf\n",m,newDist,abs(newDist - oldDist));
	}
}

//calculates the size of the file and returns no of rows
int sizeOfFile(const char  *fileName)
{
	
    	FILE *fp;
    	int cnt=0;
    	fopen_s(&fp, fileName, "r");
    	if(!fp)
        	return -1;
    	else
    	{
    		char buffer[200];
	      while(fgets(buffer, 200, fp))
        	{       		
            	cnt++;
        	}        
    	}
    	fclose(fp);
    	return cnt;
}


int _tmain(int argc, _TCHAR* argv[])
{

	//calculate no of rows in universe file i.e no of vectors with their values
    int row = sizeOfFile(filename);
    int col = VecSize,i=0;   	
    	
    // -1 indicates some error has occured while opening file
    if(row == -1)
    {
        printf("unable to open file");
        return 0;
    }

	printf("\nno of vectors in the universe file is : %d\n\n\n",row);

    //create universe array of structure vec of size row to store universe file vectors 
    vec* universe = (vec*)calloc(row, sizeof(vec));

    // NULL indicates some error has occured while opening file
    if (universe == NULL) 
	{
		printf("universe Memory not allocated using calloc..\n");
		exit(0);
	}

    //read universe file vectors in universe vector
	if(!ReadFile(universe,filename))
    {
         printf("unable to open input_files!..");
         return 0;
    }
      
	//create codebook array of structure vec of size k to store codebook file vectors
    vec* codebook = (vec*)calloc(CBSize, sizeof(vec));
      
    if (codebook == NULL) 
	{
		printf("codebook Memory not allocated using calloc..\n");
		exit(0);
	}

	K_Means(universe,codebook,row);
	

	//store the output codevectors in file
	const char *outputVector = "Codebook.txt";
	WriteFile(codebook,outputVector,CBSize);
	
	printf("\n\nFinal Codebook Vectors : \n\n");
	printVector(codebook,CBSize);
	
	printf("\n\n\n");
	
	free(universe);
	free(codebook);
	return 0;
}





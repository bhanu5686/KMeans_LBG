
#include "stdafx.h"

#define ld double 
#define delta 0.0001 
#define filename "Universe.csv"
#define VecSize 12
int CBSize = 1;
ld tokhuraWeight[12] = {1.0, 3.0, 7.0, 13.0, 19.0, 22.0, 25.0, 33.0, 42.0, 50.0, 56.0, 61.0};
ld oldDist = 0.0;
ld epsilon = 0.03;
int tempSize = 20 * CBSize;

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


//initialize codebook with centroid of the universe
void initialize(vec *universe,vec *codebook,int row)
{
	int i=0,j=0,k=0;
	for(i=0;i<1;i++)
	{
		ld tempVec[VecSize] = {0.0};	
		for(j=0;j<row;j++)
		{					
			for(k=0;k<VecSize;k++)
			{
				tempVec[k] += universe[j].data[k];
			}	
		}
				
		for(j=0;j<VecSize;j++)
		{
			codebook[i].data[j] = tempVec[j]/((ld)row);            
		}
	}
	printf("Initial Codebook : \n\n");
	printVector(codebook,CBSize);
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
				tDist = tDist + tokhura(&universe[j],&codebook[i]); 
				cnt++;
			}
		}
		total = total + (ld)cnt;
	}	
	ld avgDist = tDist/total;    
	return avgDist;
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
					temp[k] += universe[j].data[k];
				}
				cnt++;
			}	
		}
		
		for(j=0;j<VecSize;j++)
		{
			codebook[i].data[j] = temp[j]/((ld)cnt);            
		}
	}
	return;
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
		universe[i].cluster = index;
	}
	return;
}

//split codebook into double of its size and update created vectors using formula studied in the class
void SplitCodebook(vec *codebook,vec *temp,int size)
{
	int i=0,j=0,k=0,index=0;
	temp = (vec*)realloc(temp, size * sizeof(vec));
	memset(temp, 0, size * sizeof(vec));
	
	for(i=0;i<size;i++)
		temp[i] = codebook[i];
		
	int no = 2 * size;
	
	codebook = (vec*)realloc(codebook, no * sizeof(vec));
	memset(codebook, 0, no * sizeof(vec));
 
	for(i=0;i<size;i++)
	{
		index = 2*i;
		for(j=0;j<VecSize;j++)
		{
			codebook[index].data[j] = (temp[i].data[j] * (1 + epsilon));      
			codebook[index + 1].data[j] = (temp[i].data[j] * (1 - epsilon));
		}
	}
	CBSize *= 2;
	return;	
}

//kmeans algorithm
void K_Means(vec *universe,vec *codebook,int row)
{
	//initialize no of iterations
	int m=0;	

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

//LGB algorithm
void LGB(vec *universe,vec *codebook,vec *temp,int row)
{
	//initialize codebook of size 1 with centroid of universe
	initialize(universe,codebook,row);
	
	while(CBSize <= 8)
	{	
		//split codebook into double of its size and update with two vectors using epsilon
		SplitCodebook(codebook,temp,CBSize);			
		printf("\nfor Codebook of size : %d\n\n",CBSize);
		//apply k means on splitted and updated codevector
		K_Means(universe,codebook,row);		
		printf("\nGenerated Codebook of size %d is : \n\n",CBSize);	
		//print generated codebook
		printVector(codebook,CBSize);
		if(CBSize >= 8)
			break;			
	}	

	return;
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


int main() 
{
	//calculate no of rows in universe file i.e no of vectors with their value
    int row = sizeOfFile(filename);
    int col = VecSize,i=0;
    	
    printf("\nno of vectors in the universe file is : %d\n\n",row);
    	
	// -1 indicates some error has occured while opening file
    if(row == -1)
    {
        printf("unable to open file");
        return 0;
    }
      
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
      
	//create codebook array of structure vec of size k and temp array of structure to store codebook file vectors
    vec* codebook = (vec*)calloc(CBSize, sizeof(vec));
    vec* temp = (vec*)calloc(tempSize, sizeof(vec)); 
      
    if (codebook == NULL || temp == NULL) 
	{
		printf("codebook or temp Memory not allocated using calloc..\n");
		exit(0);
	}

  	LGB(universe,codebook,temp,row);
  	
	printf("\n\nFinal Codebook : \n");
	printVector(codebook,CBSize);
	
	//store the output codevectors in file
	const char *outputVector = "Codebook.txt";
	WriteFile(codebook,outputVector,CBSize);	

	printf("\n\n\n");

	free(universe);
	free(codebook);
	free(temp);
	return 0;
}

class Dictionary
{
public:

  Dictionary(char* filename)
  {
    FILE* fp;
    fp = fopen(filename, "r");
    char buffer[100];
    fgets(buffer, 100, (FILE*)fp);
    size = std::atoi(buffer);
    int i=0;
    dictionary = (char**)malloc(size*sizeof(char*));
    //printf("Made it here.\n");
    while(fgets(buffer, 100, (FILE*)fp) != NULL)
    {
      //printf("%d\n", i);
      dictionary[i] = (char*)malloc((std::strlen(buffer)+1)*sizeof(char));
      std::strcpy(dictionary[i], buffer);
      for(int j=0; j<(int)std::strlen(dictionary[i]); j++)
      {
        if(dictionary[i][j] == ' ' || dictionary[i][j] == '\t' || dictionary[i][j] == '\n')
        {
          dictionary[i][j] = '\0';
        }
      }
      //printf("%s\n", dictionary[i]);
      i++;
    }
    fclose(fp);
    //printf("dictionary file loaded.\n");
  }

  ~Dictionary()
  {
    for(int i=0; i<size; i++)
    {
      free(dictionary[i]);
    }
    free(dictionary);
  }

  int editDistance(char* word, char* word2)
  {
    //int distance;
    //printf("In ED.\n");
    int i,j,cost;
    int table[strlen(word)+1][strlen(word2)+1];

    for(i=0; i<=(int)std::strlen(word); i++)
    {
      for(j=0; j<=(int)std::strlen(word2); j++)
      {
        
        if(i==0 && j==0)
        {
          table[i][j] = 0;
        }
        else if(i==0 && j!=0)
        {
          table[i][j] = table[i][j-1]+1;
        }
        else if(i!=0 && j==0)
        {
          table[i][j] = table[i-1][j]+1;
        }
        else
        {
          if(word[i-1] == word2[j-1])
          {
            cost = 0;
          }
          else
          {
            cost = 1; 
          }
          table[i][j] = min(table[i-1][j-1]+cost, table[i-1][j]+1, table[i][j-1]+1);

          if(i>2 && j>2 && word[i-1] == word2[j-2] && word[i-2] == word2[j-1])
          {
            //printf("%d, %d\n", table[i][j], table[i-2][j-2]);
            table[i][j] = min(table[i][j], table[i][j], table[i-2][j-2]);
            //printf("%d, %d, %d\n", table[i][j], i, j);
          }
        }
      }
    }
    //printf("ED between %s and %s is %d.\n", word, word2, table[std::strlen(word)][std::strlen(word2)]);
    return table[std::strlen(word)][std::strlen(word2)];
  }

  char* spellcheck(char* input)
  {
    //printf("In spellcheck.\n");
    char line[500];
    std::strcpy(line, input);
    char* answer = (char*)malloc(500);
    int min;
    int dist;
    int currentDist = 10;
    int current;
    char word[100];
    char* token; 
    token = std::strtok(line, " ");
    //printf("token: %s.\n", token);
    /*if(search(token))
    {
      std::strcpy(answer, token);
      printf("here\n");
    }
    else
    {
      printf("there\n");
      for(int i=0; i<size; i++)
      {
        current = editDistance(dictionary[i], token);
        if(current < min)
        {
          min = current;
          std::strcpy(word, dictionary[i]);
        }
      }
      std::strcpy(answer, word);
    }*/
    while(token != NULL)
    {
      //printf("here\n");
      min = (int)std::strlen(token);
      if(search(token))
      {
        if(std::strlen(answer) != 0)
        {
          std::strcat(answer, " ");
        }
        std::strcat(answer, token);
      }
      else
      {
        //printf("there\n");
        for(int i=0; i<size; i++)
        {
          dist = (int)(std::strlen(dictionary[i]) - std::strlen(token));
          if(dist<0)
          {
            dist*=-1;
          }
          current = editDistance(dictionary[i], token);
          if(current <= min)
          {
            if(current < min || dist < currentDist)
            {
              min = current;
              std::strcpy(word, dictionary[i]);
              currentDist = dist;
            }
          }
        }
        if(std::strlen(answer) != 0)
        {
          std::strcat(answer, " ");
        }
        std::strcat(answer, word);
      }
      //printf("answer: %s...\n", answer);
      token = strtok(NULL, " ");
    }
    //printf("x\n");
    return answer;
  }



private:

  int min(int a, int b, int c)
  {
    if(a<=b && a<=c)
    {
      return a;
    }
    if(b<=c)
    {
      return b;
    }
    return c;
  }

  int search(char* word)
  {
    //printf("In search.\n");
    int low = 0;
    int high = size;
    int average;
    while(low <= high)
    {
      average = (low+high)/2;
      //printf("low: %d, high: %d, average: %d, size: %d.\n", low, high, average, size);
      if(std::strcmp(dictionary[average], word) == 0)
      {
        return 1;
      }
      else if(std::strcmp(dictionary[average], word) > 0)
      {
        high = average - 1;
      }
      else
      {
        low = average + 1;
      }
    }
    return 0;

  }

  int size;
  char** dictionary;

};


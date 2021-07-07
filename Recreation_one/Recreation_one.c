long long* divisors (long long element) {
  
  long long *sqr_div = (long long *) calloc(1, sizeof(long long));
  sqr_div[0] = 0;
  
  for (long long i = 1; i <= element; i++){
    if (element % i == 0) sqr_div[0] += i*i;
  }
  return sqr_div;
}


// fill length with the number of pairs in your array of pairs
Pair** listSquared(long long m, long long n, int* length) {
    // your code
    
    int index = 0;
    Pair **pair = (Pair **) malloc(sizeof(Pair *));
    pair[index] = (Pair *) malloc(sizeof(Pair));
    // Iterate through the range.
    for (long long i = m; i <= n; i++){
        
      // Calculate divisors for every element.
      long long *sum_div = divisors (i);
      
      if (sqrt(sum_div[0]) == floor(sqrt(sum_div[0]))){
          pair[index++][0].first = i;
          pair[index++][1].snd = sum_div[0];
          free(sum_div);
          //pair[index] = (Pair *) realloc(pair[index], sizeof(pair[index]));
      }
    }
  *length = index + 1;
  return pair;
}

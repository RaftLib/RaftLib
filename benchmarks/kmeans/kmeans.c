/* Copyright (c) 2007, Stanford University
* All rights reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:
*     * Redistributions of source code must retain the above copyright
*       notice, this list of conditions and the following disclaimer.
*     * Redistributions in binary form must reproduce the above copyright
*       notice, this list of conditions and the following disclaimer in the
*       documentation and/or other materials provided with the distribution.
*     * Neither the name of Stanford University nor the
*       names of its contributors may be used to endorse or promote products
*       derived from this software without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY STANFORD UNIVERSITY ``AS IS'' AND ANY
* EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
* DISCLAIMED. IN NO EVENT SHALL STANFORD UNIVERSITY  BE LIABLE FOR ANY
* DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
* (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
* LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
* ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
* (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
* SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/ 

#include <stdio.h>
#include <strings.h>
#include <string.h>
#include <stddef.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <string.h>
#include <math.h>
#include "stddefines.h"
#include "MapReduceScheduler.h"

#define DEF_NUM_POINTS 100000
#define DEF_NUM_MEANS 100
#define DEF_DIM 3
#define DEF_GRID_SIZE 1000

#define false 0
#define true 1

int num_points; // number of vectors
int dim;       // Dimension of each vector
int num_means; // number of clusters
int grid_size; // size of each dimension of vector space
int modified;
int num_pts = 0;

typedef struct {
   int **points;
   keyval_t *means;  // each mean is an index and a coordinate.
   int *clusters;
   int next_point;
   int unit_size;
} kmeans_data_t;

kmeans_data_t kmeans_data;

typedef struct {
   int length;
   int **points;
   keyval_t *means;
   int *clusters;
} kmeans_map_data_t;
   
     
/** dump_means()
 *  Helper function to Print out the mean values
 */   
void dump_means(keyval_t *means, int size) {
   int i,j;
   for (i = 0; i < size; i++) 
   {
      for (j = 0; j < dim; j++) 
      {
         dprintf("%5d ", ((int *)means[i].val)[j]);
      }
      dprintf("\n");
   }
      
}

/** dump_points()
 *  Helper function to print out the points
 */
void dump_points(int **vals, int rows)
{
   int i, j;
   
   for (i = 0; i < rows; i++) 
   {
      for (j = 0; j < dim; j++)
      {
         dprintf("%5d ",vals[i][j]);
      }
      dprintf("\n");
   }
}

/** parse_args()
 *  Parse the user arguments
 */
void parse_args(int argc, char **argv) 
{
   int c;
   extern char *optarg;
   extern int optind;
   
   num_points = DEF_NUM_POINTS;
   num_means = DEF_NUM_MEANS;
   dim = DEF_DIM;
   grid_size = DEF_GRID_SIZE;
   
   while ((c = getopt(argc, argv, "d:c:p:s:")) != EOF) 
   {
      switch (c) {
         case 'd':
            dim = atoi(optarg);
            break;
         case 'c':
            num_means = atoi(optarg);
            break;
         case 'p':
            num_points = atoi(optarg);
            break;
         case 's':
            grid_size = atoi(optarg);
            break;
         case '?':
            printf("Usage: %s -d <vector dimension> -c <num clusters> -p <num points> -s <max value>\n", argv[0]);
            exit(1);
      }
   }
   
   if (dim <= 0 || num_means <= 0 || num_points <= 0 || grid_size <= 0) {
      printf("Illegal argument value. All values must be numeric and greater than 0\n");
      exit(1);
   }
   
   printf("Dimension = %d\n", dim);
   printf("Number of clusters = %d\n", num_means);
   printf("Number of points = %d\n", num_points);
   printf("Size of each dimension = %d\n", grid_size);   
}

/** generate_points()
 *  Generate the points
 */
void generate_points(int **pts, int size) 
{   
   int i, j;
   
   for (i=0; i<size; i++) 
   {
      for (j=0; j<dim; j++) 
      {
         pts[i][j] = rand() % grid_size;
      }
   }
}

/** generate_means()
 *  Compute the means for the various clusters
 */
void generate_means(keyval_t *means, int size)
{
   int i, j;
   
   for (i=0; i<size; i++)
   {
      *((int *)(means[i].key)) = i; 
      
      for (j=0; j<dim; j++)
      {
         ((int *)(means[i].val))[j] = rand() % grid_size;
      }
   }   
   
}

/** get_sq_dist()
 *  Get the squared distance between 2 points
 */
inline unsigned int get_sq_dist(int *v1, int *v2)
{
   int i;
   
   unsigned int sum = 0;
   for (i = 0; i < dim; i++) 
   {
      sum += ((v1[i] - v2[i]) * (v1[i] - v2[i])); 
   }
   return sum;
}

/** add_to_sum()
 *	Helper function to update the total distance sum
 */
void add_to_sum(int *sum, int *point)
{
   int i;
   
   for (i = 0; i < dim; i++)
   {
      sum[i] += point[i];   
   }   
}

/** mykeycmp()
 *  Key comparison function
 */
int mykeycmp(const void *s1, const void *s2)
{
   if ( *((int *)s1) < *((int *)s2) ) return -1;
   else if ( *((int *)s1) > *((int *)s2) ) return 1;
   else return 0;
}

/** find_clusters()
 *  Find the cluster that is most suitable for a given set of points
 */
void find_clusters(int **points, keyval_t *means, int *clusters, int size) 
{
   int i, j;
   unsigned int min_dist, cur_dist;
   int min_idx;

   for (i = 0; i < size; i++) 
   {
      min_dist = get_sq_dist(points[i], (int *)(means[0].val));
      min_idx = 0; 
      for (j = 1; j < num_means; j++)
      {
         cur_dist = get_sq_dist(points[i], (int *)(means[j].val));
         if (cur_dist < min_dist) 
         {
            min_dist = cur_dist;
            min_idx = j;   
         }
      }

      if (clusters[i] != min_idx) 
      {
         clusters[i] = min_idx;
         modified = true;
      }
      //dprintf("Emitting [%d,%d]\n", *((int *)means[min_idx].key), *(points[i]));
      emit_intermediate(means[min_idx].key, (void *)(points[i]), sizeof(means[min_idx].key));
   }   
}


/** kmeans_splitter()
 *  
 * Assigns one or more points to each map task
 */
int kmeans_splitter(void *data_in, int req_units, map_args_t *out)
{
   kmeans_data_t *kmeans_data = (kmeans_data_t *)data_in;
   kmeans_map_data_t *out_data;
   
   assert(data_in);
   assert(out);
   assert(kmeans_data->points);
   assert(kmeans_data->means);
   assert(kmeans_data->clusters);
   assert(req_units);
   
   if (kmeans_data->next_point >= num_points) return 0;
   
   out_data = (kmeans_map_data_t *)malloc(sizeof(kmeans_map_data_t));
   out->length = 1;
   out->data = (void *)out_data;
   
   out_data->points = (void *)(&(kmeans_data->points[kmeans_data->next_point]));
   out_data->means = kmeans_data->means;
   out_data->clusters = (void *)(&(kmeans_data->clusters[kmeans_data->next_point]));
   kmeans_data->next_point += req_units;
   if (kmeans_data->next_point >= num_points) 
   {
      out_data->length = num_points - kmeans_data->next_point + req_units;
   }
   else out_data->length = req_units;   
   
   num_pts += out_data->length;
   // Return true since the out data is valid
   return 1;
}


/** kmeans_map()
 * Finds the cluster that is most suitable for a given set of points
 * 
 */
void kmeans_map(map_args_t *args) 
{
   assert(args);
   assert(args->length == 1);
   
   kmeans_map_data_t *map_data = args->data;
   find_clusters(map_data->points, map_data->means, map_data->clusters, map_data->length);  
   free(args->data);
}


/** kmeans_reduce()
 *	Updates the sum calculation for the various points
 */
void kmeans_reduce(void *key_in, void **vals_in, int vals_len) 
{
   assert(key_in);
   assert(vals_in);
   
   int i;
   int *sum;
   int *mean;
   
   sum = (int *)calloc(dim, sizeof(int));
   mean = (int *)malloc(dim * sizeof(int));
   
   for (i = 0; i < vals_len; i++)
   {
      add_to_sum(sum, vals_in[i]);
   }
   
   for (i = 0; i < dim; i++)
   {
      mean[i] = sum[i] / vals_len;
   }
   
   free(sum);
   emit(key_in, (void *)mean);
}

int main(int argc, char **argv)
{
   final_data_t kmeans_vals;
   scheduler_args_t sched_args;
   int i;
   
   parse_args(argc, argv);   
   
   // get points
   kmeans_data.points = (int **)malloc(sizeof(int *) * num_points);
   for (i=0; i<num_points; i++) 
   {
      kmeans_data.points[i] = (int *)malloc(sizeof(int) * dim);
   }
   generate_points(kmeans_data.points, num_points);
   
   // get means
   kmeans_data.means = (keyval_t *)malloc(sizeof(keyval_t) * num_means);
   for (i=0; i<num_means; i++) {
      kmeans_data.means[i].val = malloc(sizeof(int) * dim);
      kmeans_data.means[i].key = malloc(sizeof(int));
   } 
   generate_means(kmeans_data.means, num_means);
   
   kmeans_data.next_point = 0;
   kmeans_data.unit_size = sizeof(int) * dim;
 
   kmeans_data.clusters = (int *)malloc(sizeof(int) * num_points);
   memset(kmeans_data.clusters, -1, sizeof(int) * num_points);
   
   modified = true;

   // Setup scheduler args
   memset(&sched_args, 0, sizeof(scheduler_args_t));
   sched_args.task_data = &kmeans_data;
   sched_args.map = kmeans_map;
   sched_args.reduce = kmeans_reduce;
   sched_args.splitter = kmeans_splitter;
   sched_args.key_cmp = mykeycmp;
   sched_args.unit_size = kmeans_data.unit_size;
   sched_args.partition = NULL; // use default
   sched_args.result = &kmeans_vals;
   sched_args.data_size = (num_points + num_means) * dim * sizeof(int);  
   sched_args.L1_cache_size = atoi(GETENV("MR_L1CACHESIZE")); //1024 * 64;
   sched_args.num_map_threads = atoi(GETENV("MR_NUMTHREADS"));//8;
   sched_args.num_reduce_threads = atoi(GETENV("MR_NUMTHREADS"));//16;
   sched_args.num_merge_threads = atoi(GETENV("MR_NUMTHREADS")) / 2;//8;
   sched_args.num_procs = atoi(GETENV("MR_NUMPROCS"));//16;
   sched_args.key_match_factor = (float)atof(GETENV("MR_KEYMATCHFACTOR"));//2;
   
   printf("KMeans: Calling MapReduce Scheduler\n");

   while (modified == true)
   {
      modified = false;
      kmeans_data.next_point = 0;
      dprintf(".");
      CHECK_ERROR(map_reduce_scheduler(&sched_args) < 0);
      
      for (i = 0; i < kmeans_vals.length; i++)
      {
         int mean_idx = *((int *)(kmeans_vals.data[i].key));
         free(kmeans_data.means[mean_idx].val);
         kmeans_data.means[mean_idx] = kmeans_vals.data[i];
      }
      if (kmeans_vals.length > 0) free(kmeans_vals.data);
   }  
   dprintf("\n");
   printf("KMeans: MapReduce Completed\n");  

   dprintf("\n\nFinal means:\n");
   dump_means(kmeans_data.means, num_means);

   for (i = 0; i < num_points; i++) 
      free(kmeans_data.points[i]);
   free(kmeans_data.points);
   
   for (i = 0; i < num_means; i++) 
   {
      free(kmeans_data.means[i].key);
      free(kmeans_data.means[i].val);
   }
   
   //free(kmeans_vals.data);
   free(kmeans_data.clusters);

   return 0;
}

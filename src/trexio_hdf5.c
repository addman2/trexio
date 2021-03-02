/* This file was generated from the trexio.org org-mode file.
   To generate it, open trexio.org in Emacs and execute
   M-x org-babel-tangle
*/



#include "trexio_hdf5.h"

  #define NUCLEUS_GROUP_NAME  "nucleus"
  #define NUCLEUS_NUM_NAME    "nucleus_num"
  #define NUCLEUS_CHARGE_NAME "nucleus_charge"
  #define NUCLEUS_COORD_NAME  "nucleus_coord"

/* 
 * Currently H5LTread_dataset_ is used instead of this function 
 * but keep it for later if we decide to get rid of the H5LT API 
 */
dset_t* trexio_hdf5_read_dset_low(const trexio_hdf5_t* file, const char *dset_name, void *buf) {

  assert (file != NULL);
  assert (dset_name != NULL);
  assert (buf != NULL);
  /*
   * Low-level implementation. Involves dealing with all HDF5 handles and dimensions
   */
  dset_t* dset = (dset_t*) malloc(sizeof(dset_t));
  assert (dset != NULL);

  dset->dset_id = H5Dopen(file->nucleus_group, 
                          dset_name, 
                          H5P_DEFAULT);
  assert (dset->dset_id > 0);
  /*
   * Get dataspace, datatype and dimensions
   * dspace and dtype handles created below have to be closed when not used
   */    
  dset->dspace_id = H5Dget_space(dset->dset_id);
  assert (dset->dspace_id > 0);
  
  dset->dtype_id = H5Dget_type(dset->dset_id);
  assert (dset->dtype_id > 0);
  
  /* Check dimensions. Usefull, but then additional parameters 
   * ranks and dims[] have to be passed to the function
  int rrank;
  const int rank = 1;
  hsize_t dims[1] = {0};
  rrank = H5Sget_simple_extent_dims(nucleus->h5_charge->dspace_id, 
                                    dims, NULL);
  assert (rrank == rank);
  for (int i=0; i<rank; i++){
    assert (dims[i] > 0);
  }
  */
  herr_t status;
  status = H5Dread(dset->dset_id, dset->dtype_id,
               H5S_ALL, H5S_ALL, H5P_DEFAULT,
	       buf);
  assert (status >= 0);

  return dset;

}

trexio_exit_code trexio_hdf5_init(trexio_t* file) {

  trexio_hdf5_t* f = (trexio_hdf5_t*) file;

  /* If file doesn't exist, create it */
  int f_ishere = 0;
  struct stat st;
  
  if (stat(file->file_name, &st) == 0) {
    printf("%s \n","HDF5 file already exists");
   // RDWR OR RDONLY ???
    f->file_id = H5Fopen(file->file_name, H5F_ACC_RDWR, H5P_DEFAULT);
    f_ishere = 1;
  } else {
    f->file_id = H5Fcreate(file->file_name, H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT);
    f_ishere = 0;
  }  

  /* Create groups in the hdf5 file */
  if (f_ishere == 0){
    f->nucleus_group = H5Gcreate(f->file_id, NUCLEUS_GROUP_NAME, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
    //f->electron_group = H5Gcreate(f->file_id, ELECTRON_GROUP_NAME, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
  } else { 
    f->nucleus_group = H5Gopen(f->file_id, NUCLEUS_GROUP_NAME, H5P_DEFAULT);
    //f->electron_group = H5Gopen(f->file_id, ELECTRON_GROUP_NAME, H5P_DEFAULT);   
  }
  assert (f->nucleus_group > 0L);
  //assert (f->electron_group > 0L);

  return TREXIO_SUCCESS;
}

trexio_exit_code trexio_hdf5_finalize(trexio_t* file) {

  trexio_hdf5_t* f = (trexio_hdf5_t*) file;

  H5Gclose(f->nucleus_group);
  f->nucleus_group = 0;

/*
  H5Gclose(f->electron_group);
  f->electron_group = 0; 
*/

  H5Fclose(f->file_id);
  f->file_id = 0;

  return TREXIO_SUCCESS;
}

h5nucleus_t* trexio_hdf5_read_nucleus(const trexio_hdf5_t* file) {
  
  /* Allocate the data structure */
  h5nucleus_t* nucleus = (h5nucleus_t*) malloc(sizeof(h5nucleus_t));
  assert (nucleus != NULL);

  nucleus->num    = 0;
  nucleus->coord  = NULL;
  nucleus->charge = NULL;
  nucleus->h5_coord  = NULL;
  nucleus->h5_charge = NULL;

  /* Check that the file was opened/created correctly, return */ 
  if (file->file_id < 0) return nucleus;

  /* Quit if the dimensioning attribute is missing in the file */
  if (H5Aexists(file->nucleus_group, NUCLEUS_NUM_NAME) == 0) return nucleus;

  herr_t status;  
  /* Read the nucleus_num attribute of nucleus group */
  hid_t num_id;
  num_id = H5Aopen(file->nucleus_group, NUCLEUS_NUM_NAME, H5P_DEFAULT);
  assert (num_id > 0);

  status = H5Aread(num_id, H5T_NATIVE_ULLONG, &(nucleus->num));
  assert (status >= 0);

  /* Allocate and read nucleus_charge array */
  nucleus->charge = (double*) calloc(nucleus->num, sizeof(double));
  assert (nucleus->charge != NULL);

  /* High-level H5LT API. No need to deal with dataspaces and datatypes */
  status = H5LTread_dataset_double(file->nucleus_group, 
                                NUCLEUS_CHARGE_NAME, 
				nucleus->charge);

  /* Allocate and read nucleus_coord array */
  nucleus->coord = (double*) calloc(3 * nucleus->num, sizeof(double));
  assert (nucleus->coord != NULL);

  /* High-level H5LT API. No need to deal with dataspaces and datatypes */
  status = H5LTread_dataset_double(file->nucleus_group, 
                                    NUCLEUS_COORD_NAME, 
				    nucleus->coord);
  assert (status >= 0);

  /* Low-level read. Do not forget to close the associated IDs (dset,dtype,dspace) 
   *  when not used anymore, see below. Note how this function is similar to H5LTread_dataset_double
   */
  /*
  nucleus->h5_coord = trexio_hdf5_read_dset_low(file, NUCLEUS_COORD_NAME, 
  		  		nucleus->coord); 

  H5Sclose(nucleus->h5_coord->dspace_id);
  H5Tclose(nucleus->h5_coord->dtype_id);
  H5Dclose(nucleus->h5_coord->dset_id);
  */

  H5Aclose(num_id);

  return nucleus;
}


trexio_exit_code trexio_hdf5_write_nucleus(const trexio_hdf5_t* file, h5nucleus_t* nucleus) {

  assert (file != NULL);
  assert (nucleus != NULL);

  herr_t status;
  hid_t dspace, dtype; 
  hid_t attr_id;
    
  dtype = H5Tcopy(H5T_NATIVE_ULLONG);
  /* Write the dimensioning variables */
  if (H5Aexists(file->nucleus_group, NUCLEUS_NUM_NAME) == 0) {
    dspace = H5Screate(H5S_SCALAR);
    attr_id = H5Acreate(file->nucleus_group, NUCLEUS_NUM_NAME, dtype, dspace,
                     H5P_DEFAULT, H5P_DEFAULT);
    assert (attr_id > 0);

    /* High-level routine does not work for some reason
     * status = H5LTset_attribute_ulong (file->nucleus_group, "nucleus", NUCLEUS_NUM_NAME, 
     *                                  &(nucleus->num), 1);
     */
  } else {
    attr_id = H5Aopen(file->nucleus_group, NUCLEUS_NUM_NAME, H5P_DEFAULT);
    assert (attr_id > 0);
  }

  status = H5Awrite(attr_id, dtype, &(nucleus->num));
  assert (status >= 0); 

  H5Aclose(attr_id);
  
  /* Write arrays */
  hid_t dset_id;
  int charge_rank = 1;
  const hsize_t charge_dims[1] = {nucleus->num};

  if ( H5LTfind_dataset(file->nucleus_group, NUCLEUS_CHARGE_NAME) != 1) { 
    
    status = H5LTmake_dataset_double (file->nucleus_group, NUCLEUS_CHARGE_NAME, 
                                    charge_rank, charge_dims, nucleus->charge);
    assert (status >= 0);

  } else {   

    dset_id = H5Dopen(file->nucleus_group, NUCLEUS_CHARGE_NAME, H5P_DEFAULT);
    assert (dset_id > 0);
    
    dspace = H5Dget_space(dset_id);
    assert (dspace > 0);

    dtype = H5Dget_type(dset_id);
    assert (dtype > 0);

    
    int rrank;
    hsize_t dims[1] = {0};
    rrank = H5Sget_simple_extent_dims(dspace,
                                    dims, NULL);
    assert (rrank == charge_rank);
    // disabling asserts like this allows to overwrite _num variable
    for (int i=0; i<rrank; i++){
      assert (dims[i] == charge_dims[i]);
    }

    status = H5Dwrite(dset_id, H5T_NATIVE_DOUBLE, H5S_ALL, H5S_ALL, H5P_DEFAULT, nucleus->charge);
    assert (status >= 0);

    H5Sclose(dspace);
    H5Tclose(dtype);
    H5Dclose(dset_id);

  }

  int coord_rank = 2;
  const hsize_t coord_dims[2] = {nucleus->num, 3};
  if ( H5LTfind_dataset(file->nucleus_group, NUCLEUS_COORD_NAME) != 1) { 
    status = H5LTmake_dataset_double (file->nucleus_group, NUCLEUS_COORD_NAME, 
                                    coord_rank, coord_dims, nucleus->coord);
    assert (status >= 0);

  } else {   

    dset_id = H5Dopen(file->nucleus_group, NUCLEUS_COORD_NAME, H5P_DEFAULT);
    assert (dset_id > 0);
    
    dspace = H5Dget_space(dset_id);
    assert (dspace > 0);

    dtype = H5Dget_type(dset_id);
    assert (dtype > 0);

    
    int rrank;
    hsize_t dims[2] = {0, 0};
    rrank = H5Sget_simple_extent_dims(dspace,
                                    dims, NULL);
    assert (rrank == coord_rank);
    for (int i=0; i<rrank; i++){
      assert (dims[i] == coord_dims[i]);
    }

    status = H5Dwrite(dset_id, H5T_NATIVE_DOUBLE, H5S_ALL, H5S_ALL, H5P_DEFAULT, nucleus->coord);
    assert (status >= 0);

    H5Sclose(dspace);
    H5Tclose(dtype);
    H5Dclose(dset_id);

  }

  return TREXIO_SUCCESS;
}

trexio_exit_code trexio_hdf5_free_nucleus(h5nucleus_t* nucleus) {
  
  if (nucleus == NULL) return TREXIO_FAILURE;
  
  if (nucleus->coord != NULL) free (nucleus->coord);
  nucleus->coord = NULL;
  
  if (nucleus->charge != NULL) free (nucleus->charge);
  nucleus->charge = NULL;
  
  if (nucleus->h5_coord != NULL) free (nucleus->h5_coord);
  nucleus->h5_coord = NULL;
  
  if (nucleus->h5_charge != NULL) free (nucleus->h5_charge);
  nucleus->h5_charge = NULL;

  free (nucleus);

  return TREXIO_SUCCESS;
}

trexio_exit_code trexio_hdf5_read_nucleus_num(const trexio_t* file, uint64_t* num) {

  assert (file != NULL);
  assert (num  != NULL);

  h5nucleus_t* nucleus = trexio_hdf5_read_nucleus((trexio_hdf5_t*) file);
  
  if (nucleus == NULL) {
    return TREXIO_FAILURE;
  }

  /**/ *num = nucleus->num;

  trexio_hdf5_free_nucleus(nucleus);
  return TREXIO_SUCCESS;
}

 
trexio_exit_code trexio_hdf5_write_nucleus_num(const trexio_t* file, const uint64_t num) {

  assert (file != NULL);
  assert (num > 0L);
  
  h5nucleus_t* nucleus = trexio_hdf5_read_nucleus((trexio_hdf5_t*) file);

  assert (nucleus != NULL);
  
  if (nucleus->num != num) {

    if (nucleus->num != 0) {
	printf("%ld -> %ld %s \n", num, nucleus->num, 
	       "This variable alreasy exists. Overwriting it is not supported");
        trexio_hdf5_free_nucleus(nucleus);
    	return TREXIO_FAILURE;
    }

    nucleus->num = num;

    if (nucleus->charge != NULL) free(nucleus->charge);
    nucleus->charge = NULL;

    nucleus->charge = (double*) calloc(num, sizeof(double));
    assert (nucleus->charge != NULL);
    
    if (nucleus->coord  != NULL) free(nucleus->coord );
    nucleus->coord = NULL;

    nucleus->coord = (double*) calloc(3*num, sizeof(double));
    assert (nucleus->coord != NULL);

  } else {
    nucleus->num = num;
  }
  
  trexio_exit_code rc = trexio_hdf5_write_nucleus((trexio_hdf5_t*) file, nucleus);
  assert (rc == TREXIO_SUCCESS);

  trexio_hdf5_free_nucleus(nucleus);
  
  return TREXIO_SUCCESS;
}

trexio_exit_code trexio_hdf5_read_nucleus_coord(const trexio_t* file, double* coord) {

  assert (file != NULL);
  assert (coord != NULL);

  h5nucleus_t* nucleus = trexio_hdf5_read_nucleus((trexio_hdf5_t*) file);

  if (nucleus == NULL) return TREXIO_FAILURE;
  assert (nucleus->coord != NULL);
  
  for (size_t i=0 ; i<3*nucleus->num ; i++) {
    coord[i] = nucleus->coord[i];
  }

  trexio_hdf5_free_nucleus(nucleus);
  return TREXIO_SUCCESS;
}

 
trexio_exit_code trexio_hdf5_write_nucleus_coord(const trexio_t* file, const double* coord) {

  assert (file != NULL);
  assert (coord != NULL);
  
  h5nucleus_t* nucleus = trexio_hdf5_read_nucleus((trexio_hdf5_t*) file);

  if (nucleus == NULL) return TREXIO_FAILURE;
  assert (nucleus->coord != NULL); 
 
  for (size_t i=0 ; i<3*nucleus->num ; i++) {
    nucleus->coord[i] = coord[i];
  }
  
  trexio_exit_code rc = trexio_hdf5_write_nucleus((trexio_hdf5_t*) file, nucleus);
  assert (rc == TREXIO_SUCCESS);

  trexio_hdf5_free_nucleus(nucleus);
  
  return TREXIO_SUCCESS;
}
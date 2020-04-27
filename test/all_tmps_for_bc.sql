SELECT * FROM face_vectors fv
JOIN t_biocard_template_link bt ON bt.tmp_id = fv.id  AND bt.tmp_type = 17 
JOIN t_biocards bc ON bc.uid = bt.biocard_id AND bc.gid = '2c6eab8b-d050-4cbd-82d7-214d9463bbcf' 

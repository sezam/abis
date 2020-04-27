SELECT bt.* FROM t_biocards bc, t_biocard_template_link bt 
WHERE bc.gid = '2c6eab8b-d050-4cbd-82d7-214d9463bbcf' 
AND bc.uid = bt.biocard_id

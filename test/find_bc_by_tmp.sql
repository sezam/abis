SELECT bc.* FROM t_biocard_template_link bt
JOIN t_biocards bc ON bc.uid = bt.biocard_id
WHERE bt.tmp_type = 17 AND bt.tmp_id = 1 
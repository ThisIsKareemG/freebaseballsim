mysqldump -u root -p baseball --no-data=true --routines=true | sed 's/AUTO_INCREMENT=[0-9]*\b//' > schema.mysql

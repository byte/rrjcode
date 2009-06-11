#!/usr/bin/ruby -w

# gets a web page from Public Bank (page=), and then saves it into a file (appends it) to pbbrate.csv. 
# All I need is the date, the banks selling price, and the banks buying price
# Its meant to help my gold investment
# incidentally, the CSV file, is also useful for plotting, using tools like gnuplot

require 'net/http'
require 'date'

# read the page data

page = Net::HTTP.get(URI.parse('http://www.pbebank.com/en/en_content/personal/rates/goldinvest.html'))
#http = Net::HTTP.new('www.pbebank.com', 80)
#resp, page = http.get('/en/en_content/personal/rates/goldinvest.html', nil )

# BEGIN processing HTML

def parse_html(data,tag)
   return data.scan(%r{<#{tag}\s*.*?>(.*?)</#{tag}>}im).flatten
end

output = []
table_data = parse_html(page,"table")
table_data.each do |table|
   out_row = []
   row_data = parse_html(table,"tr")
   row_data.each do |row|
      cell_data = parse_html(row,"td")
      cell_data.each do |cell|
         cell.gsub!(%r{<.*?>},"")
      end
      out_row << cell_data
   end
   output << out_row
end

# END processing HTML
# examine the result

def parse_nested_array(array,tab = 0)
   n = 0
   array.each do |item|
      if(item.size > 0)
         puts "#{"\t" * tab}[#{n}] {"
         if(item.class == Array)
            parse_nested_array(item,tab+1)
         else
            puts "#{"\t" * (tab+1)}#{item}"
         end
         puts "#{"\t" * tab}}"
      end
      n += 1
   end
end

# parse_nested_array(output)

#puts `date` + "selling: " + output[32][1][1] + " buying " + output[32][1][2]
o = DateTime.now.strftime("%Y-%m-%d") + "," + output[32][1][1] + "," + output[32][1][2] + "\n"
File.open('pbbrate.csv', 'a') { |f| 
  f << o
}
<?xml version="1.0" ?>

<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">

<xsl:output method="text" />

<xsl:template match="directives">
Newsys 1.1 Documentation
By Jared Yanovich &lt;jaredy@closeedge.net&gt;
-----------------------------------------------------------------------
<xsl:apply-templates />
Newsys 1.1 Documentation
</xsl:template>

<xsl:template match="directive">
						*<xsl:value-of select="id" />*
						<xsl:value-of select="name" />
						Value type: <xsl:value-of select="type" />
<xsl:text>

</xsl:text>
<xsl:value-of select="normalize-space(description)" />
-----------------------------------------------------------------------
</xsl:template>

</xsl:stylesheet>
